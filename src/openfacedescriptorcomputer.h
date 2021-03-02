#ifndef OPENFACEDESCRIPTORCOMPUTER_H
#define OPENFACEDESCRIPTORCOMPUTER_H

#include "dnnfacedescriptorcomputer.h"
#include "openface.h"
//#include "openfacedescriptor.h"
//#include "dlibfaceextractor.h"
#include "openfaceextractor.h"

//#include <vector>
//#include <optional>
//#include <execution>
//#include <atomic>
//#include <exception>
//
//#include <dlib/opencv.h>

//// TEST!
////#include <opencv2/highgui.hpp>	
//#include <dlib/image_processing.h>
//#include <dlib/image_processing/frontal_face_detector.h>
//#include <dlib/image_io.h>


//template <>
//template <>
//DnnFaceDescriptorComputer<OpenFaceExtractor<OpenFaceAlignment::InnerEyesAndBottomLip>, OpenFace>::DnnFaceDescriptorComputer()
//	// TODO: fix paths
//	: faceExtractor("./shape_predictor_68_face_landmarks.dat", OpenFace::inputImageSize)	
//	, faceRecognizer("./nn4.v2.t7", false) {}

template <>
template <>
DnnFaceDescriptorComputer<OpenFaceExtractor<OpenFaceAlignment::OuterEyesAndNose>, OpenFace>::DnnFaceDescriptorComputer()
	// TODO: fix paths
	: faceExtractor("./shape_predictor_68_face_landmarks.dat", OpenFace::inputImageSize)
	, faceRecognizer("./nn4.v2.t7", false) {}


// OpenFace suggests using outerEyesAndNose alignment:
// https://cmusatyalab.github.io/openface/visualizations/#2-preprocess-the-raw-images
using OpenFaceDescriptorComputer = DnnFaceDescriptorComputer<OpenFaceExtractor<OpenFaceAlignment::OuterEyesAndNose>, OpenFace>;


/*
class OpenFaceDescriptorComputer //: DnnFaceDescriptorComputer<OpenFace, DlibFaceExtractor>
{
	// TODO: fix paths
	static inline const std::string faceRecognitionModel{ "./nn4.v2.t7" };	
	static inline const std::string landmarkDetectionModel{ "./shape_predictor_68_face_landmarks.dat" };

public:

	//using Descriptor = OpenFace::OutputLabel;
	//class Descriptor;
	using Descriptor = OpenFace::Descriptor;
	

	OpenFaceDescriptorComputer()
		: faceRecognizer(faceRecognitionModel)
	{	}

	// TODO: define copy/move semantics

	//template <typename Input>
	//std::optional<Descriptor> operator ()(Input&& input)
	std::optional<Descriptor> operator ()(const std::string& file)
	{
		OpenFaceExtractor faceExtractor = getFaceExtractor();
		std::optional<OpenFaceExtractor::Output> face = faceExtractor(file, OpenFace::inputImageSize);
		return face ? this->faceRecognizer(*face, false) : std::nullopt;
	}

	

	//// testing operator()(std::string)
	//std::vector<std::optional<Descriptor>> operator()(const std::vector<std::string>& files, std::size_t maxBatchSize = 64)
	//{
	//	std::vector<std::optional<Descriptor>> descriptors;
	//	for (const auto& file : files)
	//	{
	//		descriptors.push_back((*this)(file));
	//	}
	//	return descriptors;
	//}

	
	std::vector<std::optional<Descriptor>> operator()(const std::vector<std::string>& files, std::size_t maxBatchSize = 32)
	{
		std::vector<std::optional<Descriptor>> descriptors(files.size());
		auto tail = (*this)(files.cbegin(), files.cend(), descriptors.begin(), maxBatchSize);	
		assert(descriptors.end() == tail);
		return descriptors;
	}
		
	
	template <class InputIterator, class OutputIterator>
	OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead, std::size_t maxBatchSize = 32);
		

	//using Network = OpenFace;
	//using DnnFaceDescriptorComputer::FaceExtractor;

private:

	const OpenFaceExtractor& getFaceExtractor()
	{
		// TODO: should it be const?
		static OpenFaceExtractor faceExtractor{ landmarkDetectionModel };
		return faceExtractor;
	}

	OpenFace faceRecognizer;
	//DlibFaceExtractor faceExtractor;
	//OpenFaceExtractor faceExtractor;
};	// OpenFaceDescriptorComputer


// TODO: move it to _impl.h file

template <class InputIterator, class OutputIterator>
OutputIterator OpenFaceDescriptorComputer::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead, std::size_t maxBatchSize)
{
	assert(maxBatchSize > 0);
	assert(inTail >= inHead);

	//std::size_t total = inTail - inHead, completed = 0;
	std::atomic_flag eflag{ false };
	std::exception_ptr eptr;
	std::vector<std::optional<OpenFaceExtractor::Output>> faces(maxBatchSize);
	//std::vector<std::optional<Descriptor>> descriptors(inputs.size());
	std::vector<cv::Mat> inBatch(maxBatchSize), outBatch(maxBatchSize);
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
		//std::transform(std::execution::par, inHead, inTail, faces.begin(),
		std::transform(std::execution::par, inHead, batchTail, faces.begin(),
			[this, &eptr, &eflag](const auto& file) -> std::optional<OpenFaceExtractor::Output>
			{
				try
				{
					thread_local auto localExtractor = getFaceExtractor();
					return localExtractor(file, OpenFace::inputImageSize);	
				}
				catch (...)
				{
					if (!eflag.test_and_set(std::memory_order_acq_rel))
						eptr = std::current_exception();
				}

				return std::nullopt;
			});	// transform

		if (eptr)
			std::rethrow_exception(eptr);


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
		auto outBatchTail = this->faceRecognizer(inBatch.cbegin(), inBatch.cend(), outBatch.begin(), false);
		assert(outBatchTail == outBatch.end());

		outHead = std::transform(faces.cbegin(), faces.cend(), pos.cbegin(), outHead,
			[&outBatch](const std::optional<OpenFaceExtractor::Output>& face, std::size_t idx) -> std::optional<Descriptor>
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
std::vector<std::optional<Descriptor>> operator()(const std::vector<std::string>& inputs, std::size_t maxBatchSize = 2)	// TEST! batch size
{
	assert(maxBatchSize > 0);

	std::size_t total = inputs.size(), completed = 0;
	std::atomic_flag eflag{ false };
	std::exception_ptr eptr;
	std::vector<std::optional<OpenFaceExtractor::Output>> faces(maxBatchSize);
	std::vector<std::optional<Descriptor>> descriptors(inputs.size());

	for (auto head = inputs.cbegin(), tail = head; completed < total; head = tail)
	{
		auto batchSize = std::min(total - completed, maxBatchSize);
		//n -= batchSize;
		tail = head + batchSize;

		faces.resize(batchSize);
		//std::transform(std::execution::par, inputs.cbegin(), inputs.cend(), faces.begin(),
		std::transform(std::execution::par, head, tail, faces.begin(),
			[this, &eptr, &eflag](const std::string& file) -> std::optional<OpenFaceExtractor::Output>
			{
				try
				{
					thread_local auto localExtractor = this->faceExtractor;
					return localExtractor(file, 96);	// TODO: image size constant
				}
				catch (...)
				{
					if (!eflag.test_and_set(std::memory_order_acq_rel))
						eptr = std::current_exception();
				}

				return std::nullopt;
			});	// transform

		if (eptr)
			std::rethrow_exception(eptr);


		std::vector<cv::Mat> batchIn;
		std::vector<std::size_t> pos;	// idices of corresponding non-null faces/descriptors in batchIn/batchOut
		//for (auto& imOpt : inputs)
		for (std::size_t i = 0, j = 0; i < faces.size(); ++i)
		{
			pos.push_back(j);

			if (auto imOpt = faces[i]; imOpt)
			{
				batchIn.push_back(*imOpt);
				++j;
			}
			else std::cout << "empty face: " << inputs[i] << std::endl;
		}	// for i

		assert(faces.size() == batchSize);
		descriptors.resize(completed + batchSize, std::nullopt);
		if (!batchIn.empty())
		{
			auto batchOut = this->faceRecognizer(batchIn, false);
			assert(batchOut.size() == batchIn.size());

			for (std::size_t i = 0; i < faces.size(); ++i)
			{
				if (faces[i])
					descriptors[completed + i] = std::move(batchOut[pos[i]]);
					//descriptors.at(completed + i) = std::move(batchOut.at(pos.at(i)));
			}	// for
		}	// not an empty batch

		completed += batchSize;
	}	// while

	assert(completed == total);
	return descriptors;
}
*/

/*
std::vector<std::optional<Descriptor>> operator()(const std::vector<std::string>& inputs, std::size_t maxBatchSize = 100)
{
	assert(maxBatchSize > 0);

	std::size_t total = inputs.size(), completed = 0;
	std::atomic_flag eflag{ false };
	std::exception_ptr eptr;
	std::vector<std::optional<OpenFaceExtractor::Output>> faces(maxBatchSize);
	std::vector<std::optional<Descriptor>> descriptors(inputs.size());

	for (auto head = inputs.cbegin(), tail = head; completed < total; head = tail)
	{
		auto batchSize = std::min(total - completed, maxBatchSize);
		//n -= batchSize;
		tail = head + batchSize;

		faces.resize(batchSize);
		//std::transform(std::execution::par, inputs.cbegin(), inputs.cend(), faces.begin(),
		std::transform(std::execution::par, head, tail, faces.begin(),
			[this, &eptr, &eflag](const std::string& file) -> std::optional<OpenFaceExtractor::Output>
			{
				try
				{
					thread_local auto localExtractor = this->faceExtractor;
					return localExtractor(file, 96);	// TODO: image size constant
				}
				catch (...)
				{
					if (!eflag.test_and_set(std::memory_order_acq_rel))
						eptr = std::current_exception();
				}

				return std::nullopt;
			});	// transform

		if (eptr)
			std::rethrow_exception(eptr);


		assert(faces.size() == batchSize);
		descriptors.resize(completed + batchSize, std::nullopt);
		//std::vector<cv::Mat> batchIn;
		//std::vector<std::size_t> pos;	// idices of corresponding non-null faces/descriptors in batchIn/batchOut
		//for (auto& imOpt : inputs)
		for (std::size_t i = 0, j = 0; i < faces.size(); ++i)
		{
			//pos.push_back(j);

			if (auto imOpt = faces[i]; imOpt)
			{
				descriptors[completed + i] = this->faceRecognizer(*imOpt, false);
				//batchIn.push_back(*imOpt);
				++j;
			}
			else
			{
				descriptors[completed + i] = std::nullopt;
				std::cout << "empty face: " << inputs[i] << std::endl;
			}
		}	// for i



		completed += batchSize;
	}	// while

	assert(completed == total);
	return descriptors;
}
*/


#endif	// OPENFACEDESCRIPTORCOMPUTER_H