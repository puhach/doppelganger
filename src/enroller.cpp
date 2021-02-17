#include "enroller.h"

#include <iostream>	// TEST!


void Enroller::enroll(const std::string& datasetPath, const std::string& outputPath)
{
	// Initialize/reset the state variables before running the threads. Since there are no threads yet, synchronization is not used here.
	this->loadingFinished = false;	
	this->aborted = false;
	this->loadedCount = 0;

	//load(datasetPath);
	//process(outputPath);
	auto producer = std::async(std::launch::async, &Enroller::load, this, datasetPath);
	auto consumer = std::async(std::launch::async, &Enroller::process, this, outputPath);

	assert(producer.valid() && consumer.valid());	// TODO: make sure it is valid even in case of exception
	
	producer.wait();	// wait for loading to finish before setting the loadingFinished flag	
	std::unique_lock<std::mutex> lck(this->mtx);
	this->loadingFinished = true;	
	lck.unlock();	// manual unlocking is done before notifying to avoid waking up the waiting thread only to block again 
	this->bufNotEmpty.notify_one();	// wake up the consumer just in case it is still waiting for new data

	producer.get();		// may throw an exception from load()

	consumer.get();		// calls wait automatically; may throw an exception from process()
}	// enroll

void Enroller::load(const std::string& datasetPath)
{
	int index = 0, label = 0;

	for (const auto& dirEntry : std::filesystem::directory_iterator(datasetPath))
	{
		if (dirEntry.is_directory())
		{
			//std::cout << "DIRECTORY: " << dirEntry.path() << std::endl;
			for (const auto& fileEntry : std::filesystem::directory_iterator(dirEntry))
			{
				//std::cout << "\t" << fileEntry << std::endl;

				std::unique_lock<std::mutex> lckNotFull(this->mtx);

				if (this->loadedCount >= this->buf.size())	// buffer full
				{
					assert(this->loadedCount == buf.size());
					
					// Wait until the consumer frees the buffer. The lock will be released while waiting and automatically acquired 
					// again when waiting finishes. That said, the lock guards the predicate function (because technically it is not 
					// called while waiting on the conditional variable). 
					bufNotFull.wait(lckNotFull, [this]() { return this->loadedCount < this->buf.size() || this->aborted; });					
				}

				if (this->aborted)	// aborted by the user
					break;	// the lock will be automatically released

				lckNotFull.unlock();


				this->buf[index % buf.size()] = Job{ fileEntry.path(), label };
				++index;


				std::unique_lock<std::mutex> lckNotEmpty(this->mtx);
				++this->loadedCount;
				lckNotEmpty.unlock();
				bufNotEmpty.notify_one();
			}	// for fileEntry

			++label;
		}	// is directory
	}	// for dirEntry
}	// load

void Enroller::process(const std::string& outputPath)
{
	int index = 0;

	while (true)
	{
		std::unique_lock<std::mutex> lckNotEmpty(this->mtx);	

		if (this->loadedCount <= 0)
		{
			assert(this->loadedCount == 0);		// make sure we never go negative and the lock works properly
						
			// Wait until the producer fills the buffer. The lock will be released while waiting and automatically acquired 
			// again when waiting finishes. That said, the lock guards the predicate function (because technically it is not 
			// called while waiting on the conditional variable).
			bufNotEmpty.wait(lckNotEmpty, [this]() { return this->loadedCount > 0 || this->aborted || this->loadingFinished; });
		}	// nothing to process


		if (this->loadedCount <= 0)		// aborted or no more data to process
			break;	// unique_lock will release the lock automatically

		lckNotEmpty.unlock();


		// TODO: use the data
		Job&& job = std::move(this->buf[index % buf.size()]);
		std::cout << job.filePath << std::endl;
		++index;


		std::unique_lock<std::mutex> lckNotFull(this->mtx);
		--this->loadedCount;
		lckNotFull.unlock();
		bufNotFull.notify_one();
	}	// while
}	// process