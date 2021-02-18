#ifndef ENROLLER_H
#define ENROLLER_H

#include <array>
#include <fstream>
#include <filesystem>
#include <future>
#include <condition_variable>
#include <cassert>
//#include <vector>
#include <map>
#include <string>
#include <optional>

#include <dlib/matrix.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/shape_predictor.h>

//#define USE_PRODUCER_CONSUMER	

class Enroller
{
	typedef dlib::matrix<float, 0, 1> Descriptor;

public:
	//Enroller(const std::string& database, const std::string& cache = std::string());
	Enroller(const std::string& database, const std::string& landmarkDetectorPath = std::string("shape_predictor_68_face_landmarks.dat"));
	

	// TODO: define copy/move semantics

	void create(const std::string& datasetPath);

	void load(const std::string& databasePath);

	//void enroll(const std::string& datasetPath, const std::string& outputPath);

	// TODO: add the abort method

	// TODO: add the save method

private:

	struct Job
	{		
		//std::string className;
		std::filesystem::path filePath;
		int label = -1;
	};
		

#ifdef USE_PRODUCER_CONSUMER

	void listFiles(const std::string& datasetPath);

	void processFiles(const std::string& outputPath);

	//std::string datasetPath;
	std::mutex mtx;
	std::condition_variable bufNotFull, bufNotEmpty;
	int loadedCount = 0, processedCount = 0;
	bool loadingFinished = false, aborted = false;
	std::array<Job, 10> buf;	

#else	// !USE_PRODUCER_CONSUMER
	std::optional<Descriptor> computeDescriptor(const std::filesystem::path& path);
#endif	// !USE_PRODUCER_CONSUMER

	void debugMsg(const std::string& msg);

	std::mutex mtxDbg;	// TEST!		
	//std::vector<std::tuple<std::string, Descriptor>> 
	std::map<Descriptor, std::string> faceMap;
	std::vector<std::string> labels;
	const dlib::frontal_face_detector faceDetectorOrigin = dlib::get_frontal_face_detector();	// TODO: perhaps, make it static?
	dlib::shape_predictor landmarkDetector;
};	// Enroller


#endif	// ENROLLER_H