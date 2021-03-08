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



/*
* FaceExtractorHelper is an auxiliary class for face extraction. It implements functions commonly used by face extractors. 
* This class cannot be instantiated directly.
*/

//template <class OutputImage, class ExtractFaceCallback>
template <class OutputImage>
class FaceExtractorHelper
{
public:

    using Output = OutputImage;
    using ExtractFaceCallback = std::optional<Output> (FaceExtractorHelper::*)(const std::string& );

    std::optional<Output> operator()(const std::string& filePath) { return (this->*extractFaceCallback)(filePath); }

    std::optional<Output> operator()(const std::filesystem::path& filePath) { return (this->*extractFaceCallback)(filePath.string()); }

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
    ExtractFaceCallback extractFaceCallback = nullptr;
};  // FaceExtractorHelper


template <class OutputImage>
template <class InputIterator, class OutputIterator>
OutputIterator FaceExtractorHelper<OutputImage>::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead)
{
    assert(inHead <= inTail);

#ifdef PARALLEL_EXECUTION
    const auto& executionPolicy = std::execution::par;
#else
    const auto& executionPolicy = std::execution::seq;
#endif  // !PARALLEL_EXECUTION
    std::atomic_flag eflag{ false };
    std::exception_ptr eptr;
    outHead = std::transform(executionPolicy, inHead, inTail, outHead,    
        [this, &eflag, &eptr](const auto& filePath) -> std::optional<Output>
        {
            try
            {
                return (*this)(filePath);
            }
            catch (...)
            {
                // Prevent racing when multiple threads catch an exception
                if (!eflag.test_and_set(std::memory_order_acq_rel))     // noexcept
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



