#ifndef OPENFACEEXTRACTOR_H
#define OPENFACEEXTRACTOR_H


#include "faceextractorhelper.h"

#include <string>
#include <optional>
#include <exception>

#include <opencv2/core.hpp>     // cv::Mat

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include <dlib/pixel.h>
#include <dlib/opencv.h>



/*
* OpenFace preprocessing can be configured for different image alignment.
*/
enum class OpenFaceAlignment
{
    InnerEyesAndBottomLip,
    OuterEyesAndNose
};


/*
* OpenFaceExtractor crops a face detected in an input image and prepares it for face recognition by means of the OpenFace model.
* https://cmusatyalab.github.io/openface/visualizations/#2-preprocess-the-raw-images
* https://github.com/cmusatyalab/openface/blob/master/openface/align_dlib.py
*/

template <OpenFaceAlignment alignment>
class OpenFaceExtractor : FaceExtractorHelper<cv::Mat>
{
    using FaceExtractorHelper::ExtractFaceCallback;
public:
    using FaceExtractorHelper::Output;

    // OpenFaceExtractor expects a path to 68 landmark detection model 
    OpenFaceExtractor(const std::string& landmarkDetectionModel, unsigned long size)
        : FaceExtractorHelper(landmarkDetectionModel, static_cast<ExtractFaceCallback>(&OpenFaceExtractor::extractFace))
        , size(size > 0 ? size : throw std::invalid_argument("Image size cannot be zero.")) {}


    // Private inheritance prevents slicing and accidental deletion via base class pointer

    OpenFaceExtractor(const OpenFaceExtractor& other) = default;
    OpenFaceExtractor(OpenFaceExtractor&& other) = default;

    OpenFaceExtractor& operator = (const OpenFaceExtractor& other) = default;
    OpenFaceExtractor& operator = (OpenFaceExtractor&& other) = default;

    using FaceExtractorHelper::operator();

private:

    // The template for aligned facial landmarks
    static constexpr double lkTemplate[68][2] =
    {
            {0.0792396913815, 0.339223741112}, {0.0829219487236, 0.456955367943},
            {0.0967927109165, 0.575648016728}, {0.122141515615, 0.691921601066},
            {0.168687863544, 0.800341263616}, {0.239789390707, 0.895732504778},
            {0.325662452515, 0.977068762493}, {0.422318282013, 1.04329000149},
            {0.531777802068, 1.06080371126}, {0.641296298053, 1.03981924107},
            {0.738105872266, 0.972268833998}, {0.824444363295, 0.889624082279},
            {0.894792677532, 0.792494155836}, {0.939395486253, 0.681546643421},
            {0.96111933829, 0.562238253072}, {0.970579841181, 0.441758925744},
            {0.971193274221, 0.322118743967}, {0.163846223133, 0.249151738053},
            {0.21780354657, 0.204255863861}, {0.291299351124, 0.192367318323},
            {0.367460241458, 0.203582210627}, {0.4392945113, 0.233135599851},
            {0.586445962425, 0.228141644834}, {0.660152671635, 0.195923841854},
            {0.737466449096, 0.182360984545}, {0.813236546239, 0.192828009114},
            {0.8707571886, 0.235293377042}, {0.51534533827, 0.31863546193},
            {0.516221448289, 0.396200446263}, {0.517118861835, 0.473797687758},
            {0.51816430343, 0.553157797772}, {0.433701156035, 0.604054457668},
            {0.475501237769, 0.62076344024}, {0.520712933176, 0.634268222208},
            {0.565874114041, 0.618796581487}, {0.607054002672, 0.60157671656},
            {0.252418718401, 0.331052263829}, {0.298663015648, 0.302646354002},
            {0.355749724218, 0.303020650651}, {0.403718978315, 0.33867711083},
            {0.352507175597, 0.349987615384}, {0.296791759886, 0.350478978225},
            {0.631326076346, 0.334136672344}, {0.679073381078, 0.29645404267},
            {0.73597236153, 0.294721285802}, {0.782865376271, 0.321305281656},
            {0.740312274764, 0.341849376713}, {0.68499850091, 0.343734332172},
            {0.353167761422, 0.746189164237}, {0.414587777921, 0.719053835073},
            {0.477677654595, 0.706835892494}, {0.522732900812, 0.717092275768},
            {0.569832064287, 0.705414478982}, {0.635195811927, 0.71565572516},
            {0.69951672331, 0.739419187253}, {0.639447159575, 0.805236879972},
            {0.576410514055, 0.835436670169}, {0.525398405766, 0.841706377792},
            {0.47641545769, 0.837505914975}, {0.41379548902, 0.810045601727},
            {0.380084785646, 0.749979603086}, {0.477955996282, 0.74513234612},
            {0.523389793327, 0.748924302636}, {0.571057789237, 0.74332894691},
            {0.672409137852, 0.744177032192}, {0.572539621444, 0.776609286626},
            {0.5240106503, 0.783370783245}, {0.477561227414, 0.778476346951}
    };

    // Zero-based landmark indices for face alignment 
    static constexpr unsigned long innerEyesAndBottomLip[] = { 39, 42, 57 };
    static constexpr unsigned long outerEyesAndNose[] = { 36, 45, 33 };


    std::optional<Output> extractFace(const std::string& filePath);

    static Output alignFace(const cv::Mat& image, const dlib::full_object_detection& landmarks, unsigned long size);

    unsigned long size;
};  // OpenFaceExtractor




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
    const dlib::full_object_detection& landmarks, unsigned long size)
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
    constexpr auto prMinMaxX = std::minmax_element(std::cbegin(lkTemplate), std::cend(lkTemplate)
                                                , [](const auto& a, const auto& b) { return a[0] < b[0]; });
    constexpr auto prMinMaxY = std::minmax_element(std::cbegin(lkTemplate), std::cend(lkTemplate)
                                                , [](const auto& a, const auto& b) { return a[1] < b[1]; });

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



#endif	// OPENFACEEXTRACTOR_H
