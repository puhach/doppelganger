#ifndef OPENFACEEXTRACTOR_H
#error Do not include openfaceextractor_impl.h directly
#endif


#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

//#include <dlib/array2d.h>
//#include <dlib/pixel.h>
#include <dlib/opencv.h>


// TODO: consider moving this stuff to _impl.h file


inline dlib::full_object_detection OpenFaceExtractor::getLandmarks(const cv::Mat& im)		// face detection is a non-const operation
{
    dlib::cv_image<dlib::bgr_pixel> imDlib(im);

    if (auto faces = this->faceDetector(imDlib); faces.empty())		// no faces detected in this image
        return dlib::full_object_detection{};
    else
        return this->landmarkDetector(imDlib, faces.front());  // TODO: const ref?
}	// getLandmarks


template <OpenFaceExtractor::Alignment alignment>
std::optional<OpenFaceExtractor::Output> OpenFaceExtractor::operator()(const std::string& filePath, unsigned long size)
{
    cv::Mat im = cv::imread(filePath, cv::IMREAD_COLOR);
    CV_Assert(!im.empty());

    auto landmarks = getLandmarks(im);
    if (landmarks.num_parts() != std::size(lkTemplate))
        return std::nullopt;

    return alignFace<alignment>(im, landmarks, size);
}	// operator ()


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
