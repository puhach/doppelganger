#ifndef FACEDESCRIPTORCOMPUTER_H
#define FACEDESCRIPTORCOMPUTER_H

#include <optional>
#include <string>
#include <filesystem>
#include <tuple>

/*
* FaceDescriptorComputer defines a common interface and provides generic implementation for computing face descriptors.
* It is designed to be used as a base class for specific face descriptor computers and cannot be instantiated directly.
*/

template <class FaceExtractor, class FaceRecognizer>
class FaceDescriptorComputer
{
public:

	//using Descriptor = typename FaceRecognizer::OutputLabel;
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
		//return face ? this->faceRecognizer(*face) : std::nullopt;
	}

	// std::filesystem::path::string may throw implementation-defined exceptions
	std::optional<Descriptor> operator()(const std::filesystem::path& file) 
	{
		return (*this)(file.string());
	}

	std::vector<std::optional<Descriptor>> operator()(const std::vector<std::string>& files, std::size_t maxBatchSize = 64)
	{
		std::vector<std::optional<Descriptor>> descriptors(files.size());	// may throw
		auto tail = (*this)(files.cbegin(), files.cend(), descriptors.begin(), maxBatchSize);
		assert(descriptors.end() == tail);
		return descriptors;
	}

	std::vector<std::optional<Descriptor>> operator()(const std::vector<std::filesystem::path>& files, std::size_t maxBatchSize = 64)
	{
		std::vector<std::optional<Descriptor>> descriptors(files.size());	// may throw
		auto tail = (*this)(files.cbegin(), files.cend(), descriptors.begin(), maxBatchSize);
		assert(descriptors.end() == tail);
		return descriptors;
	}

	template <class InputIterator, class OutputIterator>
	OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead, std::size_t maxBatchSize);	// may throw

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
};	// FaceDescriptorComputer



template <class FaceExtractor, class FaceRecognizer>
template <class InputIterator, class OutputIterator>
OutputIterator FaceDescriptorComputer<FaceExtractor, FaceRecognizer>::operator()(InputIterator inHead, InputIterator inTail
	, OutputIterator outHead, std::size_t maxBatchSize)
{
	assert(maxBatchSize > 0);
	assert(inTail >= inHead);

	std::vector<std::optional<typename FaceExtractor::Output>> faces(maxBatchSize);
	//std::vector<std::optional<Descriptor>> descriptors(inputs.size());
	std::vector<typename FaceExtractor::Output> inBatch(maxBatchSize);
	//std::vector<Descriptor> outBatch(maxBatchSize);
	std::vector<std::optional<Descriptor>> outBatch(maxBatchSize);
	std::vector<std::size_t> pos(maxBatchSize);	// idices of corresponding items: faces -> batchIn/batchOut

	for (InputIterator batchTail; inHead < inTail; inHead = batchTail)
	{
		auto batchSize = std::min(maxBatchSize, static_cast<std::size_t>(inTail - inHead));
		batchTail = inHead + batchSize;

		faces.resize(batchSize);
		this->faceExtractor(inHead, batchTail, faces.begin());


		assert(faces.size() == batchSize);
		inBatch.clear();
		pos.clear();
		for (std::size_t i = 0, j = 0; i < faces.size(); ++i)
		{
			pos.push_back(j);	// indices of non-null faces/descriptors in batchIn/batchOut

			if (faces[i])
			{
				//inBatch.push_back(*faces[i]);
				inBatch.push_back(*std::move(faces[i]));	// a moved-from optional still contains a value, but the value itself is moved from
				++j;
			}
		}	// for i


		outBatch.resize(inBatch.size());
		auto outBatchTail = this->faceRecognizer(inBatch.cbegin(), inBatch.cend(), outBatch.begin());
		assert(outBatchTail == outBatch.end());

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



#endif	// FACEDESCRIPTORCOMPUTER_H