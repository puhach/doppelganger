#ifndef FACEDB_H
#error Do not include facedb_impl.h directly
#endif

#include <iostream>	// TEST!
#include <filesystem>
#include <execution>
//#include "facedb.h"

//FaceDb::FaceDb(const std::string& database, const std::string& landmarkDetectorPath)
template <class DescriptorComputer>
FaceDb<DescriptorComputer>::FaceDb(const std::string& database)
{
	/*dlib::deserialize(landmarkDetectorPath) >> this->landmarkDetector;
	dlib::deserialize("dlib_face_recognition_resnet_model_v1.dat") >> this->netOrigin;*/

	if (std::filesystem::is_directory(database))
		create(database);
	else
		load(database);
}	// ctor

#ifdef USE_PRODUCER_CONSUMER

void Enroller::enroll(const std::string& datasetPath, const std::string& outputPath)
{
	// Initialize/reset the state variables before running the threads. Since there are no threads yet, synchronization is not used here.
	this->loadingFinished = false;	
	this->aborted = false;
	this->loadedCount = 0;
	this->processedCount = 0;

	//load(datasetPath);
	//process(outputPath);
	auto producer = std::async(std::launch::async, &Enroller::listFiles, this, datasetPath);

	//auto consumer = std::async(std::launch::async, &FaceDb::process, this, outputPath);
	std::vector<std::future<void>> consumers(3);	// number of processing threads
	//for (int i = 0; i < consumers.size(); ++i)
	for (auto &consumer : consumers)
		consumer = std::async(std::launch::async, &Enroller::processFiles, this, outputPath);

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


void Enroller::listFiles(const std::string& datasetPath)
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
}	

void Enroller::processFiles(const std::string& outputPath)
{
	// In case of an exception we have to stop the producer(s) 
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

		Job job = std::move(this->buf[this->processedCount % buf.size()]);
		++this->processedCount;


		--this->loadedCount;
		assert(this->loadedCount >= 0);
		//debugMsg("Process: lckNotEmpty ]]");
		lckNotEmpty.unlock();
		//debugMsg("Process: lckNotEmpty ]");

		// TODO: use the data
		debugMsg(job.filePath.string());
		
		/*std::unique_lock<std::mutex> lckNotFull(this->mtx);
		--this->loadedCount;
		lckNotFull.unlock();*/
		bufNotFull.notify_one();
	}	// while
	
	completer.complete();
}	

#else	// !USE_PRODUCER_CONSUMER

template <class DescriptorComputer>
void FaceDb<DescriptorComputer>::create(const std::string& datasetPath)
{
	//namespace fs = std::filesystem;
	//this->faceMap.clear();
	this->faceDescriptors.clear();
	this->labels.clear();

	int index = 0, label = 0;

	// TODO: how does it work when datasetPath is not a directory?
	for (const auto& dirEntry : std::filesystem::directory_iterator(datasetPath))
	{
		if (dirEntry.is_directory())
		{
			std::cout << dirEntry.path() << std::endl;

			auto dirIt = std::filesystem::directory_iterator(dirEntry);
			//auto end = std::filesystem::end(dirit);
			//auto beg = std::filesystem::begin(dirit);
			//auto diff = end - beg;

			//std::for_each(std::execution::par, beg, end, [](const auto& en) {});
			std::vector<std::filesystem::path> fileEntries;
			//std::transform(beg, end, std::back_inserter(fileEntries), [](const auto& entry) { return entry.path(); });
			std::copy_if(std::filesystem::begin(dirIt), std::filesystem::end(dirIt), std::back_inserter(fileEntries),
				[](const auto& entry)
				{
					return entry.is_regular_file();
				});

			if (fileEntries.empty())	// skip empty classes
				continue;

			std::exception_ptr eptr;	// a default-constructed std::exception_ptr is a null pointer; it does not point to an exception object
			//std::mutex emtx;
			std::atomic<bool> eflag{ false };
			std::vector<std::optional<typename DescriptorComputer::Descriptor>> descriptors(fileEntries.size());
			//std::transform(std::execution::par, fileEntries.begin(), fileEntries.end(), descriptors.begin(), [](const auto& file) { return dlib::matrix<float, 0, 1>(); });
			std::transform(std::execution::par, fileEntries.begin(), fileEntries.end(), descriptors.begin(),
				[this, &eptr, &eflag](const auto& filePath) -> std::optional<typename DescriptorComputer::Descriptor>
				{
					return std::nullopt;	// TEST!
					try
					{
						thread_local DescriptorComputer descriptorComputer = getDescriptorComputer();
						// TODO: we could with some effort update thread_local variables by copying the instance of
						// the original DescriptorComputer, but how fast is that?
						//return descriptorComputer.computeDescriptor(filePath.string());
						return descriptorComputer(filePath.string());
					}	// try
					catch (...)
					{
						//std::scoped_lock lck(emtx);

						// A read-modify-write operation with this memory order is both an acquire operation and a release operation. 
						// No memory reads or writes in the current thread can be reordered before or after this store. All writes in 
						// other threads that release the same atomic variable are visible before the modification and the modification 
						// is visible in other threads that acquire the same atomic variable.
						if (!eflag.exchange(true, std::memory_order_acq_rel))	// noexcept
							eptr = std::current_exception();
						return std::nullopt;
					}
				});	// transform

			
			if (eptr)	// check whether there was an exception
				std::rethrow_exception(eptr);

			// Add the descriptors and labels to the database	

			//auto comp = [](const auto& a, const auto& b) 
			//{
			//	if (a.size() < b.size())
			//		return true;

			//	if (a.size() > b.size())
			//		return false;

			//	auto ait = a.begin();
			//	auto bit = b.begin();
			//	for (; ait != a.end() && bit != b.end();)
			//	{
			//		if (*ait < *bit)
			//			return true;
			//		if (*ait > *bit)
			//			return false;
			//	}

			//	return false;
			//};

			//std::map<ResNet::output_label_type, int, decltype(comp)> mm(comp);
			//for (auto& descriptor : descriptors)
			//{
			//	if (descriptor)
			//	{
			//		// TEST!
			//		//ResNet::output_label_type				
			//		dlib::matrix<float, 0, 1>& ref = *descriptor;
			//		mm[ref] = label;
			//		//this->faceMap[ref] = 2;
			//		//mm[ref] = 1;
			//		//mm[ref] = 2;
			//		//ds.push_back(ref);
			//		//this->faceMap[*descriptor] = label;
			//	}
			//}

			dlib::matrix<float, 0, 1> mat;
			//this->faceMap[mat] = 3;
			/*struct Hasher
			{
				std::size_t operator()(const dlib::matrix<float, 0, 1>& d) const noexcept
				{
					return 112;
				}
			};*/
			//std::unordered_map<dlib::matrix<float,0,1>, int, Hasher> testmap;
			//std::unordered_map<dlib::matrix<float, 0, 1>, int, decltype(&FaceDb::computeDescriptorHash)> testmap;
			//testmap[mat] = 3;
			this->faceMap[mat] = 3;
			for (auto& descriptor : descriptors)
			{
				if (descriptor)
				{
					auto &ref = *descriptor;
					//this->faceDescriptors.push_back(std::make_tuple(*descriptor, label));
					
					//this->faceMap.emplace(mat, label);
				}
			}
			
			this->labels.push_back(dirEntry.path().filename().string());

			++label;
		}	// is directory
	}	// for dirEntry
}	// create

template <class DescriptorComputer>
void FaceDb<DescriptorComputer>::load(const std::string& databasePath)
{
	// TODO: implement it
}	// load


#endif	// !USE_PRODUCER_CONSUMER

template<class DescriptorComputer>
const DescriptorComputer& FaceDb<DescriptorComputer>::getDescriptorComputer()
{
	// TODO: try shape_predictor_5_face_landmarks.dat
	//static DescriptorComputer descriptorComputer("./shape_predictor_68_face_landmarks.dat", "./dlib_face_recognition_resnet_model_v1.dat");	
	static DescriptorComputer descriptorComputer;
	return descriptorComputer;
}

//template <class DescriptorComputer>
//void FaceDb<DescriptorComputer>::debugMsg(const std::string& msg)
//{
//	std::scoped_lock<std::mutex> lock(this->mtxDbg);
//	std::cout << std::this_thread::get_id() << " : " << msg << std::endl;
//}

template <class DescriptorComputer>
std::size_t FaceDb<DescriptorComputer>::DescriptorHasher::operator()(typename DescriptorComputer::Descriptor const & descriptor) const noexcept
{
	return std::hash<typename DescriptorComputer::Descriptor>{}(descriptor);
	//std::hash<typename DescriptorComputer::Descriptor> h;
	//std::hash<dlib::matrix<dlib::rgb_pixel>> h;
	//return FaceDb<DescriptorComputer>::computeDescriptorHash(descriptor);
}

//template <class DescriptorComputer>
////std::size_t FaceDb<DescriptorComputer>::computeDescriptorHash(const dlib::matrix<dlib::rgb_pixel>& descriptor) noexcept
//std::size_t FaceDb<DescriptorComputer>::computeDescriptorHash(typename DescriptorComputer::Descriptor const & descriptor) noexcept
//{
//	//std::hash<dlib::matrix<dlib::rgb_pixel>> h;
//	return std::hash<typename DescriptorComputer::Descriptor>{}(descriptor);
//	//static_assert(std::is_same_v<dlib::matrix<dlib::rgb_pixel>, typename DescriptorComputer::Descriptor>);
//	//return 0;
//}

