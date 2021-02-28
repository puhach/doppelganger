#ifndef OPENFACE_H
#error Do not include openface_impl.h directly
#endif

#include <opencv2/dnn/dnn.hpp>


template <class InputIterator, class OutputIterator>
OutputIterator OpenFace::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead, bool swapRB)
{
	if (inHead == inTail)
		return outHead;

	// TODO: scale factor must be consistent with a single argument version
	//auto inBlob = cv::dnn::blobFromImages(inputs, 1 / 255.0, cv::Size(96, 96), cv::Scalar(0, 0, 0), swapRB, false, CV_32F);
	auto inBlob = cv::dnn::blobFromImages(std::vector<cv::Mat>(inHead, inTail), 1 / 255.0,
		cv::Size(inputImageSize, inputImageSize), cv::Scalar(0, 0, 0), swapRB, false, CV_32F);
	//auto inBlob = cv::dnn::blobFromImages(cv::InputArrayOfArrays(inHead, inTail), 1 / 255.0, cv::Size(96, 96), cv::Scalar(0, 0, 0), swapRB, false, CV_32F);
	net.setInput(inBlob);
	//std::vector<cv::Mat> outputBlobs;
	//net.forward(outputBlobs);
	auto outBlob = net.forward();

	//std::cout << "Output blob size: " << outBlob.rows << std::endl;

	//std::vector<OpenFace::OutputLabel> outputs{ inputs.size() };
	for (int i = 0; i < outBlob.rows; ++i)
	{
		// TODO: does it work without .clone()?
		//outputs[i] = outBlob.row(i); 
		*outHead = outBlob.row(i).clone();
		++outHead;
	}

	return outHead;
}
