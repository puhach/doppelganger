#ifndef FACEDESCRIPTORCOMPUTER_H
#define FACEDESCRIPTORCOMPUTER_H

#include <optional>
#include <string>
#include <filesystem>
#include <tuple>
#include <exception>


/*
* FaceDescriptorComputer defines a common interface and provides generic implementation for computing face descriptors.
* It is designed to be used as a base class for specific face descriptor computers and cannot be instantiated directly.
*/

template <class FaceExtractor, class FaceRecognizer>
class FaceDescriptorComputer
{
public:

	using Descriptor = typename FaceRecognizer::Descriptor;


	std::optional<Descriptor> operator ()(const std::string& file) noexcept(
		std::is_nothrow_invocable_v<FaceExtractor, const std::string&> &&
		std::is_nothrow_invocable_v<FaceRecognizer, typename FaceExtractor::Output&&> &&
		// std::optional throws any exception thrown by the constructor of T
		std::is_nothrow_move_constructible_v<typename FaceExtractor::Output> && 
		std::is_nothrow_move_constructible_v<Descriptor>)
	{
		std::optional<typename FaceExtractor::Output> face = this->faceExtractor(file);
		return face ? this->faceRecognizer(*std::move(face)) : std::nullopt;
	}
		
	std::optional<Descriptor> operator()(const std::filesystem::path& file) 
	{	
		return (*this)(file.string());	// std::filesystem::path::string may throw implementation-defined exceptions
	}
	
	std::vector<std::optional<Descriptor>> operator()(const std::vector<std::string>& files)
	{
		std::vector<std::optional<Descriptor>> descriptors(files.size());	// may throw
		auto tail = (*this)(files.cbegin(), files.cend(), descriptors.begin());
		assert(descriptors.end() == tail);
		return descriptors;
	}

	std::vector<std::optional<Descriptor>> operator()(const std::vector<std::filesystem::path>& files)
	{
		std::vector<std::optional<Descriptor>> descriptors(files.size());	// may throw
		auto tail = (*this)(files.cbegin(), files.cend(), descriptors.begin());
		assert(descriptors.end() == tail);
		return descriptors;
	}

	template <class InputIterator, class OutputIterator>
	OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead);	// may throw

	std::size_t getMaxBatchSize() const noexcept { return this->maxBatchSize; }
    
	void setMaxBatchSize(std::size_t maxBatchSize) 
	{ 
		this->maxBatchSize = maxBatchSize>0 ? maxBatchSize : throw std::invalid_argument("The batch size must be positive."); 
	}
    
protected:

	// Suppress copying, moving, and deletion of descendants through a pointer or reference to this class. It also prevents slicing.

	// There seems to be no guarantee that std::make_from_tuple<> never throws an exception
	template <typename... FaceExtractorArgs, typename... FaceRecognizerArgs>
	FaceDescriptorComputer(std::tuple<FaceExtractorArgs...> faceExtractorArgs, std::tuple<FaceRecognizerArgs...> faceRecognizerArgs)
		: faceExtractor(std::make_from_tuple<FaceExtractor>(std::move(faceExtractorArgs)))
		, faceRecognizer(std::make_from_tuple<FaceRecognizer>(std::move(faceRecognizerArgs))) {}

	~FaceDescriptorComputer() = default;

	FaceDescriptorComputer(const FaceDescriptorComputer& other) = default;
	FaceDescriptorComputer(FaceDescriptorComputer && other) = default;

	FaceDescriptorComputer& operator = (const FaceDescriptorComputer & other) = default;
	FaceDescriptorComputer& operator = (FaceDescriptorComputer && other) = default;
    
private:

	FaceExtractor faceExtractor;
	FaceRecognizer faceRecognizer;
	std::size_t maxBatchSize = 64;
};	// FaceDescriptorComputer



template <class FaceExtractor, class FaceRecognizer>
template <class InputIterator, class OutputIterator>
OutputIterator FaceDescriptorComputer<FaceExtractor, FaceRecognizer>::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead)
{
	assert(inTail >= inHead);

	std::vector<std::optional<typename FaceExtractor::Output>> faces(this->maxBatchSize);
	std::vector<typename FaceExtractor::Output> inBatch(this->maxBatchSize);
	std::vector<std::optional<Descriptor>> outBatch(this->maxBatchSize);
	std::vector<std::size_t> pos(this->maxBatchSize);	// idices of corresponding items: faces -> batchIn/batchOut

	for (InputIterator batchTail; inHead < inTail; inHead = batchTail)
	{
		// Make sure that the batch boundaries never exceed the range of the input sequence
		auto batchSize = std::min(this->maxBatchSize, static_cast<std::size_t>(inTail - inHead));
		batchTail = inHead + batchSize;

		// Preprocess input images and extract faces
		faces.resize(batchSize);
		this->faceExtractor(inHead, batchTail, faces.begin());

		// Select successfully extracted faces to prepare an input batch for face recognition. For each input file keep the corresponding
		// position of a face in the input batch (which is the same as the position of a face descriptor in the output batch).
		assert(faces.size() == batchSize);
		inBatch.clear();
		pos.clear();
		for (std::size_t i = 0, j = 0; i < faces.size(); ++i)
		{
			pos.push_back(j);	// indices of non-null faces/descriptors in batchIn/batchOut

			if (faces[i])
			{
				inBatch.push_back(*std::move(faces[i]));	// a moved-from optional still contains a value, but the value itself is moved from
				++j;
			}
		}	// for i

		// Recognize the faces
		outBatch.resize(inBatch.size());
		auto outBatchTail = this->faceRecognizer(inBatch.cbegin(), inBatch.cend(), outBatch.begin());
		assert(outBatchTail == outBatch.end());

		// Arrange the computed face descriptors according to the input files
		outHead = std::transform(faces.cbegin(), faces.cend(), pos.cbegin(), outHead,
			[&outBatch](const std::optional<typename FaceExtractor::Output>& face, std::size_t idx) -> std::optional<Descriptor>
			{
				assert(!face || idx < outBatch.size());
				return face ? std::move(outBatch[idx]) : std::optional<Descriptor>(std::nullopt);
			});

	}	// while

	assert(inHead == inTail);
	return outHead;
}	// operator ()


/*
* FaceDescriptorComputer is not meant to be used directly, therefore DescriptorComputerType is only forward-declared but not defined here.
*/
template <typename T>
struct DescriptorComputerType;



#endif	// FACEDESCRIPTORCOMPUTER_H
