#ifndef RESNETFACEDESCRIPTORCOMPUTER_H
#define RESNETFACEDESCRIPTORCOMPUTER_H

#include "resnet.h"
#include "dnnfacedescriptorcomputer.h"
#include "dlibfaceextractor.h"

//using ResNetFaceDescriptorComputer = DnnFaceDescriptorComputer<ResNet, DlibFaceExtractor>;

/*
* ResNetFaceDescriptorComputer is a default-constructible class for computing face descriptors using ResNet neural network.
* The face is cropped from an input image by means of DlibFaceExtractor. 
* 
* Private inheritance prevents slicing and accidental deletion via base class pointer. On the other hand, when subclassed
* privately, ResNetFaceDescriptorComputer is not treated as a type of DnnFaceDescriptorComputer<ResNet, DlibFaceExtractor>, 
* so we won't be able to pass it to a function taking a pointer/reference to DnnFaceDescriptorComputer<ResNet, DlibFaceExtractor>. 
* Alternatively, we could define a virtual copy interface and make DnnFaceDescriptorComputer destructor virtual, but that
* would lead to performance overhead.
*/

template <>
template <>
DnnFaceDescriptorComputer<DlibFaceExtractor, ResNet>::DnnFaceDescriptorComputer()
	// TODO: try 5-landmark detector
	: faceExtractor("./shape_predictor_68_face_landmarks.dat", ResNet::inputImageSize, 0.25)
	, faceRecognizer("./dlib_face_recognition_resnet_model_v1.dat") {}


using ResNetFaceDescriptorComputer = DnnFaceDescriptorComputer<DlibFaceExtractor, ResNet>;

/*
class ResNetFaceDescriptorComputer
{
	// TODO: try 5-landmark detector
	static inline const std::string faceRecognitionModel{ "./dlib_face_recognition_resnet_model_v1.dat" };
	static inline const std::string landmarkDetectionModel{ "./shape_predictor_68_face_landmarks.dat" };

public:

	using Descriptor = ResNet::OutputLabel;

	ResNetFaceDescriptorComputer()
		: faceExtractor(landmarkDetectionModel, ResNet::inputImageSize, 0.25)
		, faceRecognizer(faceRecognitionModel) {}

	// TODO: define copy/move semantics

	std::optional<Descriptor> operator ()(const std::string& file)
	{
		//DlibFaceExtractor faceExtractor = getFaceExtractor();
		std::optional<DlibFaceExtractor::Output> face = this->faceExtractor(file);
		return face ? this->faceRecognizer(*face) : std::nullopt;
	}

	std::vector<std::optional<Descriptor>> operator()(const std::vector<std::string>& files, std::size_t maxBatchSize = 32)
	{
		std::vector<std::optional<Descriptor>> descriptors(files.size());
		auto tail = (*this)(files.cbegin(), files.cend(), descriptors.begin(), maxBatchSize);
		assert(descriptors.end() == tail);
		return descriptors;
	}


	template <class InputIterator, class OutputIterator>
	OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead, std::size_t maxBatchSize = 32);

private:
	DlibFaceExtractor faceExtractor;
	ResNet faceRecognizer;
};	// ResNetFaceDescriptorComputer



template <class InputIterator, class OutputIterator>
OutputIterator ResNetFaceDescriptorComputer::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead, std::size_t maxBatchSize)
{
	assert(maxBatchSize > 0);
	assert(inTail >= inHead);

	//std::size_t total = inTail - inHead, completed = 0;
	
	std::vector<std::optional<DlibFaceExtractor::Output>> faces(maxBatchSize);
	//std::vector<std::optional<Descriptor>> descriptors(inputs.size());
	std::vector<DlibFaceExtractor::Output> inBatch(maxBatchSize);
	std::vector<ResNet::OutputLabel> outBatch(maxBatchSize);
	std::vector<std::size_t> pos(maxBatchSize);	// idices of corresponding items: faces -> batchIn/batchOut

	//for (InputIterator batchTail; completed < total; inHead = inTail)
	for (InputIterator batchTail; inHead < inTail; inHead = batchTail)
	{
		//auto batchSize = std::min(total - completed, maxBatchSize);
		auto batchSize = std::min(maxBatchSize, static_cast<std::size_t>(inTail - inHead));
		//n -= batchSize;
		//inTail = inHead + batchSize;
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
				inBatch.push_back(*faces[i]);
				++j;
			}
		}	// for i


		//std::vector<cv::Mat> batchOut(batchIn.size());
		outBatch.resize(inBatch.size());
		auto outBatchTail = this->faceRecognizer(inBatch.cbegin(), inBatch.cend(), outBatch.begin());
		assert(outBatchTail == outBatch.end());

		outHead = std::transform(faces.cbegin(), faces.cend(), pos.cbegin(), outHead,
			[&outBatch](const std::optional<DlibFaceExtractor::Output>& face, std::size_t idx) -> std::optional<Descriptor>
			{
				assert(idx < outBatch.size());
				return face ? outBatch[idx] : std::optional<Descriptor>(std::nullopt);
			});

	}	// while

	//assert(completed == total);
	assert(inHead == inTail);
	//return descriptors;
	return outHead;
}	// operator ()
*/



/*
class ResNetFaceDescriptorComputer : DnnFaceDescriptorComputer<ResNet, DlibFaceExtractor>	// TODO: make it final?
{
    // TODO: try 5-landmark detector
	static inline const std::string faceRecognitionModel{ "./dlib_face_recognition_resnet_model_v1.dat" };
	static inline const std::string landmarkDetectionModel{ "./shape_predictor_68_face_landmarks.dat" };
public:
	// This is an alternative to std::is_nothrow_constructible (slightly shorter in this case). 
	// Since we don't care about the destructor, we use "new" rather than T(arg). Also it's a global placement new
	// (thus we ignore memory allocation exceptions). In many implementations is_nothrow_constructible is 
	// effectively noexcept(T(arg)) though.
	ResNetFaceDescriptorComputer() noexcept(noexcept(::new (nullptr) DnnFaceDescriptorComputer(faceRecognitionModel, landmarkDetectionModel)))
		: DnnFaceDescriptorComputer(faceRecognitionModel, DlibFaceExtractor{ landmarkDetectionModel, ResNet::inputImageSize, 0.25 })
	{ }

	
	// TODO: define copy/move semantics

	using DnnFaceDescriptorComputer::operator ();

	using DnnFaceDescriptorComputer::Network;
	using DnnFaceDescriptorComputer::FaceExtractor;
	using DnnFaceDescriptorComputer::Descriptor;

private:
	
};	// ResNetFaceDescriptorComputer
*/

#endif	// RESNETFACEDESCRIPTORCOMPUTER_H
