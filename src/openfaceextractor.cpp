#include "openfaceextractor.h"

//#include <dlib/image_io.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include <dlib/opencv.h>


OpenFaceExtractor::OpenFaceExtractor(const std::string& landmarkDetectionModel)
{
	dlib::deserialize(landmarkDetectionModel) >> this->landmarkDetector;
}

std::optional<OpenFaceExtractor::Output> OpenFaceExtractor::operator()(const std::string& filePath, unsigned long size)
{
    // TODO: perhaps, it makes sense to add a DlibFaceDetector class which would take a path to a file and return dlib::rectangle 
    // Or maybe even better a landmark detector since we don't use the rectangle

    //dlib::matrix<dlib::rgb_pixel> im;	// TODO: array or matrix
    //dlib::load_image(im, filePath);
    cv::Mat im = cv::imread(filePath, cv::IMREAD_COLOR);
    CV_Assert(!im.empty());

    dlib::cv_image<dlib::bgr_pixel> imDlib(im);
    auto faces = this->faceDetector(imDlib);
    if (faces.empty())		// no faces detected in this image
        return std::nullopt;

    dlib::rectangle faceRect = faces[0];

    //dlib::cv_image<dlib::bgr_pixel> dlibIm{ im };
    //auto landmarks = this->landmarkDetector(dlibIm, r);
    auto landmarks = this->landmarkDetector(imDlib, faceRect);  // TODO: const ref?
    if (landmarks.num_parts() < 1)
        return std::nullopt;


    int innerEyesAndBottomLip[] = { 39, 42, 57 };
    int outerEyesAndNose[] = { 36, 45, 33 };

    const int(&ids)[3] = outerEyesAndNose;

    std::vector<cv::Point2f> inPts{ 3 };
    //dlibLandmarksToPoints(landmarks, inPts);
    for (int i = 0; i < 3; ++i)
    {
        inPts[i].x = landmarks.part(ids[i]).x();
        inPts[i].y = landmarks.part(ids[i]).y();
    }


    double coords[68][2] = {
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

    double minx = 1000, maxx = 0, miny = 1000, maxy = 0;
    for (int i = 0; i < 68; ++i)
    {
        minx = std::min(minx, coords[i][0]);
        maxx = std::max(maxx, coords[i][0]);
        miny = std::min(miny, coords[i][1]);
        maxy = std::max(maxy, coords[i][1]);
    }

    //double padding = 0.25;
    double padding = 0.3;
    double w = size / (2 * padding + 1), h = w;

    std::vector<cv::Point2f> outPts{ 3 };
    for (int i = 0; i < 3; ++i)
    {
        double x = coords[ids[i]][0];
        double y = coords[ids[i]][1];

        x = (x - minx) / (maxx - minx);
        y = (y - miny) / (maxy - miny);

        //x *= size;
        //y *= size;
        //x = size*padding + w*x;
        //y = size*padding + h*y;
        x = w * padding + w * x;
        y = h * padding + h * y;

        outPts[i].x = x;
        outPts[i].y = y;
    }

    
    //cv::Mat t = cv::estimateAffine2D(inPts, outPts);
    cv::Mat t = cv::getAffineTransform(inPts, outPts);

    cv::Mat alignedFace;
    cv::warpAffine(im, alignedFace, t, cv::Size(size, size));

    return alignedFace;
}