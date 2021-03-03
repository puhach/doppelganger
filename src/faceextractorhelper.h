#ifndef FACEEXTRACTORHELPER_H
#define FACEEXTRACTORHELPER_H

#include <functional>
#include <optional>
#include <atomic>
#include <execution>
#include <exception>
#include <string>
#include <filesystem>

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

    std::optional<Output> operator()(const std::string& filePath) { return extractFaceCallback(filePath); }

    std::optional<Output> operator()(const std::filesystem::path& filePath) { return extractFaceCallback(filePath.string()); }

    template <class InputIterator, class OutputIterator>
    OutputIterator operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead);

protected:

    // This class is auxiliary and is not supposed to be directly constructed
    FaceExtractorHelper(const std::string& landmarkDetectionModel, ExtractFaceCallback&& extractFaceCallback)
        : extractFaceCallback(std::move(extractFaceCallback))
    {
        dlib::deserialize(landmarkDetectionModel) >> this->landmarkDetector;
    }

    FaceExtractorHelper(const FaceExtractorHelper& other) = default;
    FaceExtractorHelper(FaceExtractorHelper&& other) = default;
    
    FaceExtractorHelper& operator = (const FaceExtractorHelper& other) = default;
    FaceExtractorHelper& operator = (FaceExtractorHelper&& other) = default;

    template <class DlibImage>
    dlib::full_object_detection getLandmarks(DlibImage&& image);		// face detection is a non-const operation

private:

    static const dlib::frontal_face_detector& getFaceDetector()
    {
        static const dlib::frontal_face_detector faceDetector = dlib::get_frontal_face_detector();
        return faceDetector;
    }

    // Dlib's shape predictor is thread-safe:
    // http://dlib.net/dlib/image_processing/shape_predictor_abstract.h.html
    dlib::shape_predictor landmarkDetector;
    //std::function<std::optional<Output>(const std::string& filePath)> extractFaceCallback;
    ExtractFaceCallback extractFaceCallback;
    //unsigned long size;
};  // FaceExtractorHelper

//template <class OutputImage>
//std::optional<OutputImage> FaceExtractorHelper<OutputImage>::operator()(const std::string& filePath)
//{
//    return extractFaceCallback(filePath);
//}
//
//template <class OutputImage>
//std::optional<OutputImage> FaceExtractorHelper<OutputImage>::operator()(const std::filesystem::path& filePath)
//{
//    return 
//}

template <class OutputImage>
template <class InputIterator, class OutputIterator>
OutputIterator FaceExtractorHelper<OutputImage>::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead)
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
                // Prevent racing when multiple threads catch an exception
                if (!eflag.test_and_set(std::memory_order_acq_rel))
                    eptr = std::current_exception();
            }

            return std::nullopt;
        });	// transform

    if (eptr)
        std::rethrow_exception(eptr);

    return outHead;
}   // operator ()

template <class OutputImage>
template <class DlibImage>
dlib::full_object_detection FaceExtractorHelper<OutputImage>::getLandmarks(DlibImage&& image)	// face detection is a non-const operation
{
    thread_local auto faceDetector = getFaceDetector();     // shared by all instances running in the same thread

    if (auto faces = faceDetector(image); faces.empty())		// no faces detected in this image
        return dlib::full_object_detection{};
    else
        return this->landmarkDetector(std::forward<DlibImage>(image), faces.front());  // the landmark detector is thread-safe
}   // getLandmarks

#endif	// FACEEXTRACTORHELPER_H



