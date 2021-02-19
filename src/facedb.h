#ifndef FACEDB_H
#define FACEDB_H


//#include "resnet.h"		// TODO: remove it when a separate face recognizer is used

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

//#include <dlib/matrix.h>
//#include <dlib/image_processing/frontal_face_detector.h>
//#include <dlib/image_processing/shape_predictor.h>

//#define USE_PRODUCER_CONSUMER	

template <class DescriptorComputer>
class FaceDb
{
	//typedef dlib::matrix<float, 0, 1> Descriptor;	// TODO: this is ResNet::output_label_type
	static_assert(std::is_default_constructible_v<DescriptorComputer>);
public:
	//FaceDb(const std::string& database, const std::string& cache = std::string());
	FaceDb(const std::string& database);
	

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
	//std::optional<DescriptorComputer::Descriptor> computeDescriptor(const std::filesystem::path& path);
#endif	// !USE_PRODUCER_CONSUMER

	// This singleton is intended to be used in conjunction with thread-local variables.
	// It allows us to create one instance of DescriptorComputer and then duplicate it when necessary
	// (thus, we don't need to deserialize models again). An important consequence of it is that we can't easily 
	// update the DescriptorComputer without affecting other instances.
	static const DescriptorComputer& getDescriptorComputer();	

	void debugMsg(const std::string& msg);

	std::mutex mtxDbg;	// TEST!		
	std::vector<std::tuple<typename DescriptorComputer::Descriptor, int>> faceDescriptors;
	//std::map<Descriptor, std::string> faceMap;	// TODO: give a try to unordered_map
	std::vector<std::string> labels;
	//const dlib::frontal_face_detector faceDetectorOrigin = dlib::get_frontal_face_detector();	// TODO: perhaps, make it static?
	//dlib::shape_predictor landmarkDetector;
	//ResNet netOrigin;
	//static const DescriptorComputer descriptorComputerOrigin;
};	// FaceDb

#include "facedb_impl.h"

#endif	// FACEDB_H