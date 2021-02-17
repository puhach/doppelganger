#ifndef ENROLLER_H
#define ENROLLER_H

#include <array>
#include <fstream>
#include <filesystem>
#include <future>
#include <condition_variable>
#include <cassert>


class Enroller
{
public:
	//Enroller(const std::string& datasetPath)
	//	: datasetPath(datasetPath)
	//	//: std::filesystem::directory_iterator(datasetPath, )
	//{

	//}

	// TODO: define copy/move semantics

	void enroll(const std::string& datasetPath, const std::string& outputPath);

	// TODO: add the abort method

private:

	struct Job
	{		
		//std::string className;
		std::filesystem::path filePath;
		int label = -1;
	};

	void load(const std::string& datasetPath);

	void process(const std::string& outputPath);

	//std::string datasetPath;
	std::mutex mtx;
	std::condition_variable bufNotFull, bufNotEmpty;
	int loadedCount = 0;
	bool loadingFinished = false, aborted = false;
	std::array<Job, 10> buf;
};	// Enroller


#endif	// ENROLLER_H