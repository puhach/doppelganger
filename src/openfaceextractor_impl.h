#ifndef OPENFACEEXTRACTOR_H
#error Do not include openfaceextractor_impl.h directly
#endif


#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

//#include <dlib/array2d.h>
//#include <dlib/pixel.h>
#include <dlib/opencv.h>


template <OpenFaceAlignment alignment>
std::optional<typename OpenFaceExtractor<alignment>::Output> OpenFaceExtractor<alignment>::extractFace(const std::string& filePath)
{
    cv::Mat im = cv::imread(filePath, cv::IMREAD_COLOR);
    CV_Assert(!im.empty());

    dlib::cv_image<dlib::bgr_pixel> imDlib(im);
    auto landmarks = FaceExtractorHelper::getLandmarks(imDlib);
    if (landmarks.num_parts() != std::size(lkTemplate))
        return std::nullopt;

    return alignFace(im, landmarks, this->size);
}

template <OpenFaceAlignment alignment>
typename OpenFaceExtractor<alignment>::Output OpenFaceExtractor<alignment>::alignFace(const cv::Mat& image,
    const dlib::full_object_detection& landmarks, unsigned long size) const
{
    // Depending on the alignment parameter, refer to the landmark indices for inner eye corners and a bottom lip or outer eye corners and a nose
    constexpr const unsigned long(&lkIds)[3] = (alignment == OpenFaceAlignment::InnerEyesAndBottomLip ? innerEyesAndBottomLip : outerEyesAndNose);

    // Extract the coordinates of the landmarks we are interested in
    std::array<cv::Point2f, std::size(lkIds)> inPts;
    for (std::size_t i = 0; i < inPts.size(); ++i)
    {
        inPts[i].x = landmarks.part(lkIds[i]).x();
        inPts[i].y = landmarks.part(lkIds[i]).y();
    }

    // Find the boundaries in the template of relative landmark coordinates
    constexpr auto prMinMaxX = std::minmax_element(std::cbegin(lkTemplate), std::cend(lkTemplate), [](const auto& a, const auto& b) { return a[0] < b[0]; });
    constexpr auto prMinMaxY = std::minmax_element(std::cbegin(lkTemplate), std::cend(lkTemplate), [](const auto& a, const auto& b) { return a[1] < b[1]; });

    // Scale the coordinates from the template to the output image size (this way we obtain target coordinates for face alignment)
    std::array<cv::Point2f, std::size(lkIds)> outPts;   // unfortunately, cv::Point2f has no constexpr constructor
    std::transform(std::cbegin(lkIds), std::cend(lkIds), outPts.begin(),
        [size, minX = (*prMinMaxX.first)[0], maxX = (*prMinMaxX.second)[0], minY = (*prMinMaxY.first)[1], maxY = (*prMinMaxY.second)[1]](auto lkId)
    {
        return cv::Point2f(size * (lkTemplate[lkId][0] - minX) / (maxX - minX), size * (lkTemplate[lkId][1] - minY) / (maxY - minY));
    });


    // Use 3 selected pairs of points to compute a transformation matrix 
    cv::Mat t = cv::getAffineTransform(inPts, outPts);

    // Align the face image by means of the computed transformation matrix
    cv::Mat alignedFace;
    cv::warpAffine(image, alignedFace, t, cv::Size(size, size));

    return alignedFace;
}   // alignFace

/*


inline dlib::full_object_detection OpenFaceExtractor::getLandmarks(const cv::Mat& im)		// face detection is a non-const operation
{
    thread_local auto faceDetector = getFaceDetector();
    dlib::cv_image<dlib::bgr_pixel> imDlib(im);

    if (auto faces = faceDetector(imDlib); faces.empty())		// no faces detected in this image
        return dlib::full_object_detection{};
    else
        return this->landmarkDetector(imDlib, faces.front());  // TODO: const ref?
}	// getLandmarks


template <OpenFaceExtractor::Alignment alignment>   // TODO: make it a class template parameter since there is no way to use it in generic context
//std::optional<OpenFaceExtractor::Output> OpenFaceExtractor::operator()(const std::string& filePath, unsigned long size)
std::optional<OpenFaceExtractor::Output> OpenFaceExtractor::operator()(const std::string& filePath)
{
    cv::Mat im = cv::imread(filePath, cv::IMREAD_COLOR);
    CV_Assert(!im.empty());

    auto landmarks = getLandmarks(im);
    if (landmarks.num_parts() != std::size(lkTemplate))
        return std::nullopt;

    return alignFace<alignment>(im, landmarks, this->size);
}	// operator ()


// TODO: much code is duplicated, perhaps it makes sense to inherit OpenFaceExtractor and DlibFaceExtractor from AbstractDlibExtractor

template <class InputIterator, class OutputIterator>
OutputIterator OpenFaceExtractor::operator()(InputIterator inHead, InputIterator inTail, OutputIterator outHead)
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



// This function was translated from the official OpenFace module for Dlib-based alignment:
// https://github.com/cmusatyalab/openface/blob/master/openface/align_dlib.py
template <OpenFaceExtractor::Alignment alignment>
OpenFaceExtractor::Output OpenFaceExtractor::alignFace(const cv::Mat& im, const dlib::full_object_detection& landmarks, unsigned long size) const
{
    // Depending on the alignment parameter, refer to the landmark indices for inner eye corners and a bottom lip or outer eye corners and a nose
    constexpr const unsigned long(&lkIds)[3] = (alignment == Alignment::InnerEyesAndBottomLip ? innerEyesAndBottomLip : outerEyesAndNose);

    // Extract the coordinates of the landmarks we are interested in
    std::array<cv::Point2f, std::size(lkIds)> inPts;
    for (std::size_t i = 0; i < inPts.size(); ++i)
    {
        inPts[i].x = landmarks.part(lkIds[i]).x();
        inPts[i].y = landmarks.part(lkIds[i]).y();
    }

    // Find the boundaries in the template of relative landmark coordinates
    constexpr auto prMinMaxX = std::minmax_element(std::cbegin(lkTemplate), std::cend(lkTemplate), [](const auto& a, const auto& b) { return a[0] < b[0]; });
    constexpr auto prMinMaxY = std::minmax_element(std::cbegin(lkTemplate), std::cend(lkTemplate), [](const auto& a, const auto& b) { return a[1] < b[1]; });

    // Scale the coordinates from the template to the output image size (this way we obtain target coordinates for face alignment)
    std::array<cv::Point2f, std::size(lkIds)> outPts;   // unfortunately, cv::Point2f has no constexpr constructor
    std::transform(std::cbegin(lkIds), std::cend(lkIds), outPts.begin(),
        [size, minX = (*prMinMaxX.first)[0], maxX = (*prMinMaxX.second)[0], minY = (*prMinMaxY.first)[1], maxY = (*prMinMaxY.second)[1]](auto lkId)
    {
        return cv::Point2f(size * (lkTemplate[lkId][0] - minX) / (maxX - minX), size * (lkTemplate[lkId][1] - minY) / (maxY - minY));
    });


    // Use 3 selected pairs of points to compute a transformation matrix 
    cv::Mat t = cv::getAffineTransform(inPts, outPts);

    // Align the face image by means of the computed transformation matrix
    cv::Mat alignedFace;
    cv::warpAffine(im, alignedFace, t, cv::Size(size, size));

    return alignedFace;
}	// alignFace
*/