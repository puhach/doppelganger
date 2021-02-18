#include "enroller.h"

#include <iostream>	// TEST!
#include <dlib/image_io.h>
#include <dlib/image_transforms.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/dnn.h>
#include <dlib/gui_widgets.h>	// TEST!

void Enroller::enroll(const std::string& datasetPath, const std::string& outputPath)
{
	// Initialize/reset the state variables before running the threads. Since there are no threads yet, synchronization is not used here.
	this->loadingFinished = false;	
	this->aborted = false;
	this->loadedCount = 0;
	this->processedCount = 0;

	//load(datasetPath);
	//process(outputPath);
	auto producer = std::async(std::launch::async, &Enroller::load, this, datasetPath);

	//auto consumer = std::async(std::launch::async, &Enroller::process, this, outputPath);
	std::vector<std::future<void>> consumers(3);	// number of processing threads
	//for (int i = 0; i < consumers.size(); ++i)
	for (auto &consumer : consumers)
		consumer = std::async(std::launch::async, &Enroller::process, this, outputPath);

	//assert(producer.valid() && consumer.valid());	// TODO: make sure it is valid even in case of exception
	assert(producer.valid());

	//producer.wait();	// wait for loading to finish before setting the loadingFinished flag	
	//std::unique_lock<std::mutex> lck(this->mtx);
	//this->loadingFinished = true;	
	//lck.unlock();	// manual unlocking is done before notifying to avoid waking up the waiting thread only to block again 
	////this->bufNotEmpty.notify_one();	// wake up the consumer just in case it is still waiting for new data
	//this->bufNotEmpty.notify_all();	// wake up the consumers just in case they are still waiting for new data

	producer.get();		// may throw an exception from load()

	//consumer.get();		// calls wait automatically; may throw an exception from process()
	for (auto& consumer : consumers)
	{
		assert(consumer.valid());
		consumer.get();
	}
}	// enroll

void Enroller::load(const std::string& datasetPath)
{
	// In case of an exception we need to let the consumer know that it has to stop also
	struct Completer
	{
		Completer(Enroller& enroller) noexcept
			: enroller(enroller) {}

		~Completer() noexcept(false)	// a throwing destructor must be explicitly declared noexcept(false)
		{
			std::unique_lock<std::mutex> lck(enroller.mtx);		// scoped_lock may throw
			if (this->completed)
				enroller.loadingFinished = true;
			else
				enroller.aborted = true;
			lck.unlock();

			enroller.bufNotEmpty.notify_all();	// wake up the consumers just in case they are still waiting for new data
		}

		void complete() noexcept { this->completed = true; }

		Enroller& enroller;
		bool completed = false;
	} completer(*this);

	//dlib::frontal_face_detector faceDetector = dlib::get_frontal_face_detector();	// TODO: move to the class
	//dlib::shape_predictor landmarkDetector;
	//dlib::deserialize("./shape_predictor_68_face_landmarks.dat") >> landmarkDetector;

	int index = 0, label = 0;

	for (const auto& dirEntry : std::filesystem::directory_iterator(datasetPath))
	{
		if (dirEntry.is_directory())
		{
			//std::cout << "DIRECTORY: " << dirEntry.path() << std::endl;
			for (const auto& fileEntry : std::filesystem::directory_iterator(dirEntry))
			{
				//std::cout << "\t" << fileEntry << std::endl;
				//debugMsg("Load: lckNotFull [");
				std::unique_lock<std::mutex> lckNotFull(this->mtx);

				if (this->loadedCount >= this->buf.size())	// buffer full
				{
					assert(this->loadedCount == buf.size());
					
					// Wait until the consumer frees the buffer. The lock will be released while waiting and automatically acquired 
					// again when waiting finishes. That said, the lock guards the predicate function (because technically it is not 
					// called while waiting on the conditional variable). 
					bufNotFull.wait(lckNotFull, [this]() { return this->loadedCount < this->buf.size() || this->aborted; });					
					//// TEST!
					//if (!bufNotFull.wait_for(lckNotFull,
					//	std::chrono::seconds(3),
					//	[this]() { return this->loadedCount < this->buf.size() || this->aborted; }))
					//{
					//	debugMsg("timeout");
					//	bufNotFull.wait(lckNotFull, [this]() { return this->loadedCount < this->buf.size() || this->aborted; });
					//}

				}

				if (this->aborted)	// aborted by an exception or a user 
				{
					//debugMsg("Load: break");
					break;	// the lock will be automatically released
				}

				lckNotFull.unlock();
				//debugMsg("Load: lckNotFull ]");

				/*
				// Load the image
				dlib::array2d<dlib::rgb_pixel> im;
				dlib::load_image(im, fileEntry.path().string());

				// Detect the face
				// TODO: downsample the image
				dlib::pyramid_down<2> pyrDown;		// TODO: common
				dlib::array2d<dlib::rgb_pixel> imDown;	// TODO: common?
				//if (double scale = std::max(im.nr(), im.nc()) / 300.0; scale > 1)
				bool downsampled = false;
				if (std::max(im.nr(), im.nc()) > 300)
				{
					pyrDown(im, imDown);
					im.swap(imDown);
					downsampled = true;
				}

				auto faces = faceDetector(im);
				if (faces.empty())		// no faces detected in this image
					continue;

				dlib::rectangle faceRect = faces[0];
				//if (imDown.begin() != imDown.end())		// scale back
				if (downsampled)
				{
					im.swap(imDown);
					faceRect = pyrDown.rect_up(faceRect);
				}

				//auto landmarks = landmarkDetector(im, faces[0]);	// TODO: remember to scale back the face rectangle
				auto landmarks = landmarkDetector(im, faceRect);
				if (landmarks.num_parts() < 1)	
					continue;

				// Align the face
				dlib::matrix<dlib::rgb_pixel> face;		// TODO: matrix vs array2d
				dlib::extract_image_chip(im, dlib::get_face_chip_details(landmarks, 256, 0.25), face);	// TODO: add class parameters

				//dlib::image_window win(face, "test");
				//win.wait_until_closed();
				*/

				// Add to the queue
				this->buf[index % buf.size()] = Job{ fileEntry.path(), label };
				++index;

				
				//debugMsg("Load: lckNotEmpty [");
				std::unique_lock<std::mutex> lckNotEmpty(this->mtx);
				++this->loadedCount;
				lckNotEmpty.unlock();
				//debugMsg("Load: lckNotEmpty ]");
				bufNotEmpty.notify_one();
			}	// for fileEntry

			++label;
		}	// is directory
	}	// for dirEntry

	//debugMsg("Load finished");
	completer.complete();
}	// load

void Enroller::process(const std::string& outputPath)
{
	struct Completer
	{
		Completer(Enroller& enroller) noexcept
			: enroller(enroller) {}

		~Completer() noexcept(false)	// a throwing destructor must be explicitly declared noexcept(false)
		{
			if (!this->completed)
			{
				std::unique_lock<std::mutex> lck(enroller.mtx);	// may throw
				enroller.aborted = true;
				lck.unlock();

				enroller.bufNotFull.notify_all();	// notify the producer(s)
			}
		}

		void complete() noexcept { this->completed = true; }

		Enroller& enroller;
		bool completed = false;
	} completer(*this);

	//int index = 0;

	while (true)
	{
		//debugMsg("Process: lckNotEmpty [");
		std::unique_lock<std::mutex> lckNotEmpty(this->mtx);
		//std::unique_lock<std::mutex> lckNotEmpty(this->mtx, std::chrono::seconds(3));	// TEST!
		//if (!lckNotEmpty.owns_lock())
		//{
		//	debugMsg("Process: lckNotEmpty timeout");
		//	lckNotEmpty.lock();
		//}
		//debugMsg("Process: lckNotEmpty [[");

		if (this->loadedCount <= 0 && !this->aborted && !this->loadingFinished)
		{
			assert(this->loadedCount == 0);		// make sure we never go negative and the lock works properly

			// Wait until the producer fills the buffer. The lock will be released while waiting and automatically acquired 
			// again when waiting finishes. That said, the lock guards the predicate function (because technically it is not 
			// called while waiting on the conditional variable).
			bufNotEmpty.wait(lckNotEmpty, [this]() { return this->loadedCount > 0 || this->aborted || this->loadingFinished; });
			//// TEST!
			//if (!bufNotEmpty.wait_for(lckNotEmpty, std::chrono::seconds(3),
			//	[this]() { return this->loadedCount > 0 || this->aborted || this->loadingFinished; }))
			//{
			//	debugMsg("Process: timeout");
			//	bufNotEmpty.wait(lckNotEmpty, [this]() { return this->loadedCount > 0 || this->aborted || this->loadingFinished; });
			//}
		}	// nothing to process

		if (this->loadedCount <= 0)		// aborted or no more data to process
		{
			//debugMsg("break");
			break;	// unique_lock will release the lock automatically
		}

		// TODO: use the data
		Job job = std::move(this->buf[this->processedCount % buf.size()]);
		++this->processedCount;


		--this->loadedCount;
		assert(this->loadedCount >= 0);
		//debugMsg("Process: lckNotEmpty ]]");
		lckNotEmpty.unlock();
		//debugMsg("Process: lckNotEmpty ]");

		// TODO: use the data
		debugMsg(job.filePath.string());
		//std::cout << job.filePath << std::endl;
		//Job&& job = std::move(this->buf[index % buf.size()]);
		//std::cout << job.filePath << std::endl;
		//++index;
		// TEST! 
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));
		
		/*std::unique_lock<std::mutex> lckNotFull(this->mtx);
		--this->loadedCount;
		lckNotFull.unlock();*/
		bufNotFull.notify_one();
	}	// while
	
	completer.complete();
}	// process

void Enroller::debugMsg(const std::string& msg)
{
	std::scoped_lock<std::mutex> lock(this->mtxDbg);
	std::cout << std::this_thread::get_id() << " : " << msg << std::endl;
}