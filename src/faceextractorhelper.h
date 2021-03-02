#ifndef FACEEXTRACTORHELPER_H
#define FACEEXTRACTORHELPER_H

#include <functional>
#include <optional>
#include <atomic>
#include <execution>
#include <exception>
#include <string>

#include <dlib/image_io.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/shape_predictor.h>


//template <class OutputImage, class ExtractFaceCallback>
template <class OutputImage>
class FaceExtractorHelper
{
public:

    using Output = OutputImage;
    using ExtractFaceCallback = std::function<std::optional<Output>(const std::string&)>;

    FaceExtractorHelper(const std::string& landmarkDetectionModel, ExtractFaceCallback&& extractFaceCallback)
        : extractFaceCallback(std::move(extractFaceCallback)) 
    {
        dlib::deserialize(landmarkDetectionModel) >> this->landmarkDetector;
    }

    // TODO: define copy/move semantics

    std::optional<Output> operator()(const std::string& filePath);

    template <class InputIterator, class OutputIterator>
    OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead);

protected:

    template <class DlibImage>
    dlib::full_object_detection getLandmarks(DlibImage&& image);		// face detection is a non-const operation

private:

    static const dlib::frontal_face_detector& getFaceDetector()
    {
        static const dlib::frontal_face_detector faceDetector = dlib::get_frontal_face_detector();
        return faceDetector;
    }

    //dlib::frontal_face_detector faceDetector = dlib::get_frontal_face_detector();
    dlib::shape_predictor landmarkDetector;
    //std::function<std::optional<Output>(const std::string& filePath)> extractFaceCallback;
    ExtractFaceCallback extractFaceCallback;
    //unsigned long size;
};  // FaceExtractorHelper

template <class Output>
std::optional<Output> FaceExtractorHelper<Output>::operator()(const std::string& filePath)
{
    return extractFaceCallback(filePath);
    //Image im = imageLoader(filePath);

    //auto landmarks = landmarkDetector(im);

    ////return alignFace<alignment>(im, landmarks, this->size);
    //return faceAligner(im, landmarks);
}

template <class Output>
template <class InputIterator, class OutputIterator>
OutputIterator FaceExtractorHelper<Output>::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead)
{
    assert(inHead <= inTail);

    std::atomic_flag eflag{ false };
    std::exception_ptr eptr;
    outHead = std::transform(std::execution::par, inHead, inTail, outHead,
        [this, &eflag, &eptr](const auto& filePath) -> std::optional<Output>
        {
            try
            {
                return (*this)(filePath);
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

    return outHead;
}

template <class Output>
template <class DlibImage>
dlib::full_object_detection FaceExtractorHelper<Output>::getLandmarks(DlibImage&& image)		// face detection is a non-const operation
{
    thread_local auto faceDetector = getFaceDetector();

    if (auto faces = faceDetector(image); faces.empty())		// no faces detected in this image
        return dlib::full_object_detection{};
    else
        return this->landmarkDetector(std::forward<DlibImage>(image), faces.front());  // TODO: const ref?
}

#endif	// FACEEXTRACTORHELPER_H



