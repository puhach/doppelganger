#include "enroller.h"

void Enroller::enroll(const std::string& datasetPath, const std::string& outputPath)
{
	load(datasetPath);
	process(outputPath);
}	// enroll

void Enroller::load(const std::string& datasetPath)
{
	auto loader = std::async(std::launch::async, [this, &datasetPath]()
	{
		for (const auto& dirEntry : std::filesystem::directory_iterator(datasetPath))
		{
			if (dirEntry.is_directory())
			{
				//std::cout << "DIRECTORY: " << dirEntry.path() << std::endl;
				for (const auto& fileEntry : std::filesystem::directory_iterator(dirEntry))
				{
					//std::cout << "\t" << fileEntry << std::endl;
				}	// for fileEntry
			}	// is directory
		}	// for dirEntry
	});	// async

		//loader.wait();
	assert(loader.valid());
	loader.get();
}	// load

void Enroller::process(const std::string& outputPath)
{

}	// process