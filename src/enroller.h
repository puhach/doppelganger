#ifndef ENROLLER_H
#define ENROLLER_H

#include <fstream>
#include <filesystem>
#include <future>
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

private:

	void load(const std::string& datasetPath);

	void process(const std::string& outputPath);

	//std::string datasetPath;

};	// Enroller


#endif	// ENROLLER_H