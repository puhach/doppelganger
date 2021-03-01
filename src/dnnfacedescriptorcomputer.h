#ifndef DNNDESCRIPTORCOMPUTER_H
#define DNNDESCRIPTORCOMPUTER_H

#include <optional>
#include <string>

template <class FaceExtractor, class FaceRecognizer>
class DnnFaceDescriptorComputer
{
public:

	using Descriptor = typename FaceRecognizer::OutputLabel;

	template <typename ... Args>
	DnnFaceDescriptorComputer(Args&& ... args);		// the constructor is left to be defined by specializations
		

	// TODO: define copy/move semantics

	std::optional<Descriptor> operator ()(const std::string& file)
	{
		std::optional<typename FaceExtractor::Output> face = this->faceExtractor(file);
		return face ? this->faceRecognizer(*face) : std::nullopt;
	}

	std::vector<std::optional<Descriptor>> operator()(const std::vector<std::string>& files, std::size_t maxBatchSize = 64)
	{
		std::vector<std::optional<Descriptor>> descriptors(files.size());
		auto tail = (*this)(files.cbegin(), files.cend(), descriptors.begin(), maxBatchSize);
		assert(descriptors.end() == tail);
		return descriptors;
	}


	template <class InputIterator, class OutputIterator>
	OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead, std::size_t maxBatchSize);

private:
	FaceExtractor faceExtractor;
	FaceRecognizer faceRecognizer;
};	// DnnFaceDescriptorComputer


template <class FaceExtractor, class FaceRecognizer>
template <class InputIterator, class OutputIterator>
OutputIterator DnnFaceDescriptorComputer<FaceExtractor, FaceRecognizer>::operator()(InputIterator inHead, InputIterator inTail
	, OutputIterator outHead, std::size_t maxBatchSize)
{
	assert(maxBatchSize > 0);
	assert(inTail >= inHead);

	std::vector<std::optional<typename FaceExtractor::Output>> faces(maxBatchSize);
	//std::vector<std::optional<Descriptor>> descriptors(inputs.size());
	std::vector<typename FaceExtractor::Output> inBatch(maxBatchSize);
	std::vector<Descriptor> outBatch(maxBatchSize);
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
				assert(idx < outBatch.size());
				return face ? std::move(outBatch[idx]) : std::optional<Descriptor>(std::nullopt);
			});

	}	// while

	assert(inHead == inTail);
	return outHead;
}	// operator ()



/*
// TODO: perhaps, it would be meaningful to rename it to GenericFaceDescriptorComputer<Algorithm, FaceExtractor>

template <class NetworkType, class FaceExtractorType>
class DnnFaceDescriptorComputer
{
	// TODO: check whether the network is callable rather 
	//static_assert(std::is_convertible_v<typename FaceExtractorType::Output, typename NetworkType::Input>, "FaceExtractor output type must be compatible with Network input type.");
public:
	using Network = NetworkType;
	using FaceExtractor = FaceExtractorType;
	using Descriptor = typename Network::OutputLabel;

	template <class NetworkT, class FaceExtractorT>
	DnnFaceDescriptorComputer(NetworkT&& network, FaceExtractorT&& faceExtractor) noexcept(std::is_nothrow_constructible_v<Network, NetworkT>
																					&& std::is_nothrow_constructible_v<FaceExtractor, FaceExtractorT>)
		: network(std::forward<NetworkT>(network))
		, faceExtractor(std::forward<FaceExtractorT>(faceExtractor))
	{

	}

	// TODO: define copy/move semantics

	template <typename Input>
	//std::optional<typename Network::Descriptor> compute(Input&& input)
	std::optional<Descriptor> operator ()(Input&& input)
	{
		std::optional<typename FaceExtractor::Output> face = faceExtractor(std::forward<Input>(input));
		return face ? network(*std::move(face)) : std::nullopt;
	}

private:
	FaceExtractor faceExtractor;
	Network network;
};	// DnnFaceDescriptorComputer


*/



#endif	//DNNDESCRIPTORCOMPUTER_H