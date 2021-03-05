#ifndef FACEDB_H
#error Do not include facedb_impl.h directly
#endif

#include <iostream>	// TEST!
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <execution>
#include <atomic>
#include <optional>
//#include "facedb.h"


//template <class DescriptorComputer, class DescriptorMetric>
//FaceDb<DescriptorComputer, DescriptorMetric>::FaceDb(Reporter&& reporter)
//	: reporter(std::move(reporter)) { }		// std::function's move contructor may throw until C++20
//
////FaceDb::FaceDb(const std::string& database, const std::string& landmarkDetectorPath)
//template <class DescriptorComputer, class DescriptorMetric>
//FaceDb<DescriptorComputer, DescriptorMetric>::FaceDb(const std::string& database, Reporter&& reporter)
//	: reporter(std::move(reporter))
//{
//	if (std::filesystem::is_directory(database))
//		create(database);
//	else
//		load(database);
//}	// ctor



//template <class DescriptorComputer, class DescriptorMetric>
//void FaceDb<DescriptorComputer, DescriptorMetric>::create(const std::string& datasetPath)
//{
//	//namespace fs = std::filesystem;
//	this->faceMap.clear();
//	//this->faceDescriptors.clear();
//	this->labels.clear();
//
//	std::size_t label = 0;
//
//	for (const auto& dirEntry : std::filesystem::directory_iterator(datasetPath))
//	{
//		if (dirEntry.is_directory())
//		{
//			std::cout << dirEntry.path() << std::endl;
//
//			auto dirIt = std::filesystem::directory_iterator(dirEntry);
//
//			std::vector<std::filesystem::path> fileEntries;
//			std::copy_if(std::filesystem::begin(dirIt), std::filesystem::end(dirIt), std::back_inserter(fileEntries),
//				[](const auto& entry)
//				{
//					return entry.is_regular_file();
//				});
//
//			if (fileEntries.empty())	// skip empty classes
//				continue;
//
//			// TEST!
//			std::vector<std::string> strEntries{ fileEntries.size() };
//			std::transform(fileEntries.cbegin(), fileEntries.cend(), strEntries.begin(), [](const auto& path) { return path.string(); });
//			thread_local DescriptorComputer descriptorComputer = getDescriptorComputer();
//			auto descriptors = descriptorComputer(strEntries);
//
//			/*
//			std::exception_ptr eptr;	// a default-constructed std::exception_ptr is a null pointer; it does not point to an exception object
//			//std::mutex emtx;
//			std::atomic<bool> eflag{ false };
//			std::vector<std::optional<Descriptor>> descriptors(fileEntries.size());
//			std::transform(std::execution::seq, fileEntries.begin(), fileEntries.end(), descriptors.begin(),	// TEST!
//				[this, &eptr, &eflag](const auto& filePath) -> std::optional<Descriptor>
//				{
//					try
//					{
//						// For performance reasons descriptor computer is not configurable: it will be default-constructed for all instances
//						// of FaceDb template specialized with the same descriptor computer type and copied for each thread only once.
//						// We could with some effort update thread_local variables by copying the modifiable instance of the original 
//						// descriptor computer, but it still would be rather slow. 
//						thread_local DescriptorComputer descriptorComputer = getDescriptorComputer();						
//						return descriptorComputer(filePath.string());
//					}	// try
//					catch (...)
//					{
//						//std::scoped_lock lck(emtx);
//
//						// A read-modify-write operation with this memory order is both an acquire operation and a release operation. 
//						// No memory reads or writes in the current thread can be reordered before or after this store. All writes in 
//						// other threads that release the same atomic variable are visible before the modification and the modification 
//						// is visible in other threads that acquire the same atomic variable.
//						if (!eflag.exchange(true, std::memory_order_acq_rel))	// noexcept
//							eptr = std::current_exception();
//						return std::nullopt;
//					}
//				});	// transform
//
//			
//			if (eptr)	// check whether there was an exception
//				std::rethrow_exception(eptr);
//				*/
//
//			// Add the descriptors and labels to the database	
//
//			for (auto& descriptor : descriptors)
//			{
//				if (descriptor)
//				{
//					this->faceMap.emplace_back(*std::move(descriptor), label);
//				}
//			}
//			
//			this->labels.push_back(dirEntry.path().filename().string());
//			
//			++label;
//		}	// is directory
//	}	// for dirEntry
//}	// create



template <class DescriptorComputer, class DescriptorMetric>
void FaceDb<DescriptorComputer, DescriptorMetric>::create(const std::string& datasetPath)
{
	this->reporter("Creating the database from " + datasetPath);

	this->faceMap.clear();
	this->labels.clear();

	std::size_t label = 0;
	std::vector<std::filesystem::path> fileEntries;
	std::vector<std::size_t> fileLabels;

	// Scan the dataset directory adding files to the processing list and subfolder names to the list of labels
	for (const auto& dirEntry : std::filesystem::directory_iterator(datasetPath))
	{
		if (dirEntry.is_directory())
		{
			auto dirIt = std::filesystem::directory_iterator(dirEntry);			
			std::copy_if(std::filesystem::begin(dirIt), std::filesystem::end(dirIt), std::back_inserter(fileEntries),
				[](const auto& entry) {	return entry.is_regular_file();	});
						
			std::fill_n(std::back_inserter(fileLabels), fileEntries.size()-fileLabels.size(), label);
			
			this->labels.push_back(dirEntry.path().filename().string());	
			++label;
		}	// is directory
	}	// for dirEntry

	assert(fileEntries.size() == fileLabels.size());

	// Compute descriptors for each file
	this->reporter("Processing " + std::to_string(fileEntries.size()) + " files in " + std::to_string(this->labels.size()) + " directories...");
	auto descriptors = this->descriptorComputer(fileEntries);

	// Add the descriptors and labels to the database	
	this->faceMap.reserve(descriptors.size());
	for (std::size_t i = 0; i < descriptors.size(); ++i)
	{
		if (descriptors[i])
			this->faceMap.emplace_back(*std::move(descriptors[i]), fileLabels[i]);	// constexpr T&& optional::operator*() &&
	}

	this->reporter("The database has been created.");
}	// create


/*
template <class DescriptorComputer, class DescriptorMetric>
void FaceDb<DescriptorComputer, DescriptorMetric>::create(const std::string& datasetPath)
{
	this->reporter("Creating the database from " + datasetPath);

	this->faceMap.clear();
	this->labels.clear();

	std::size_t label = 0;
	//std::vector<std::pair<std::filesystem::path, std::size_t>> fileEntries;
	//std::vector<std::pair<>>
	std::vector<std::filesystem::path> fileEntries;
	std::vector<std::size_t> fileLabels;

	for (const auto& dirEntry : std::filesystem::directory_iterator(datasetPath))
	{
		if (dirEntry.is_directory())
		{
			//std::cout << dirEntry.path() << std::endl;
						
			
			auto dirIt = std::filesystem::directory_iterator(dirEntry);
			std::copy_if(std::filesystem::begin(dirIt), std::filesystem::end(dirIt), std::back_inserter(fileEntries),
				[](const auto& entry) {	return entry.is_regular_file();	});

			std::fill_n(std::back_inserter(fileLabels), fileEntries.size() - fileLabels.size(), label);
									
			this->labels.push_back(dirEntry.path().filename().string());
			++label;
		}	// is directory
	}	// for dirEntry

	assert(fileEntries.size() == fileLabels.size());
	
	//reporter(std::string("Found ").append(std::to_string(fileEntries.size())).append(" in ").append(std::to_string(this->labels.size())).append(" directories."));
	this->reporter("Processing " + std::to_string(fileEntries.size()) + " files in " + std::to_string(this->labels.size()) + " directories...");

	std::exception_ptr eptr;	// a default-constructed std::exception_ptr is a null pointer; it does not point to an exception object
	std::atomic<bool> eflag{ false };
	std::vector<std::pair<std::optional<Descriptor>, std::size_t>> descriptors(fileEntries.size());
	std::transform(std::execution::par, fileEntries.begin(), fileEntries.end(), fileLabels.begin(), descriptors.begin(),
	//std::transform(std::execution::seq, fileEntries.begin(), fileEntries.end(), fileLabels.begin(), descriptors.begin(),	// TEST!
		[this, &eptr, &eflag](const auto& filePath, auto label) -> std::pair<std::optional<Descriptor>, std::size_t>
		{
			if (!eflag.load(std::memory_order_acquire))
			{
				try
				{
					// For performance reasons descriptor computer is not configurable: it will be default-constructed for all instances
					// of FaceDb template specialized with the same descriptor computer type and copied for each thread only once.
					// We could with some effort update thread_local variables by copying the modifiable instance of the original 
					// descriptor computer, but it still would be rather slow. 
					thread_local DescriptorComputer descriptorComputer = getDescriptorComputer();
					return std::make_pair(descriptorComputer(filePath.string()), label);
				}	// try
				catch (...)
				{
					// Atomically set the eflag to true and check whether it was not set before to avoid a race.  
					// A read-modify-write operation with memory_order_acq_rel memory order is both an acquire operation and a release  
					// operation. No memory reads or writes in the current thread can be reordered before or after this store. All writes
					// in other threads that release the same atomic variable are visible before the modification and the modification 
					// is visible in other threads that acquire the same atomic variable.
					if (!eflag.exchange(true, std::memory_order_acq_rel))	// noexcept
						eptr = std::current_exception();
				}	// catch
			}	// !eflag

			return std::make_pair(std::nullopt, label);
		});	// transform


	if (eptr)	// check whether there was an exception
		std::rethrow_exception(eptr);


	// Add the descriptors and labels to the database	

	this->faceMap.reserve(descriptors.size());		// reserve memory to avoid reallocation (might be slightly more than needed)
	for (auto& [descriptor, label] : descriptors)
	{
		// TODO: descriptor must be move-constructible
		if (descriptor)
			this->faceMap.emplace_back(*std::move(descriptor), label);	// constexpr T&& optional::operator*() &&
	}

	this->reporter("The database has been created.");
}	// create
*/


template <class DescriptorComputer, class DescriptorMetric>
void FaceDb<DescriptorComputer, DescriptorMetric>::load(const std::string& databasePath)
{
	try
	{
		this->reporter("Loading the database from " + databasePath);

		std::ifstream db(databasePath, std::ios::in);

		// Set the mask of error states on occurrence of which the stream throws an exception of type failure
		// (EOF is also included because we need all the data, not just a part of it)
		db.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);

		std::size_t numLabels;
		db >> numLabels;

		this->labels.resize(numLabels);
		for (std::size_t i = 0; i < numLabels; ++i)
			db >> std::quoted(this->labels[i]);		// read each label string removing quotes

		std::size_t numDescriptors = 0;
		db >> numDescriptors;
		this->faceMap.clear();
		this->faceMap.reserve(numDescriptors);
		for (std::size_t i = 0; i < numDescriptors; ++i)
		{
			Descriptor d;		// descriptors must be default-constructible
			std::size_t label;
			db >> d >> label;		// descriptors must be deserializable by means of >> operator
			this->faceMap.emplace_back(std::move(d), label);	// add the descriptor and the label to the map
		}	// i

		this->reporter("The database has been loaded.");
	}	// try
	catch (const std::ios_base::failure& e)
	{
		throw std::ios_base::failure("Failed to load the database file " + databasePath, e.code());
	}
}	// load


template <class DescriptorComputer, class DescriptorMetric>
void FaceDb<DescriptorComputer, DescriptorMetric>::save(const std::string& databasePath)
{
	try
	{
		this->reporter("Saving the database to " + databasePath);

		std::ofstream db(databasePath, std::ios::out);
		db.exceptions(std::ios_base::badbit | std::ios_base::failbit);

		db << this->labels.size() << std::endl;
		//std::copy(this->labels.begin(), this->labels.end(), std::ostream_iterator<std::string>(db, "\n"));
		for (const auto& label : labels)
		{			
			db << std::quoted(label) << std::endl;		// quote the labels just in case there is a space
		}

		db << this->faceMap.size() << std::endl;
		for (const auto& [descriptor, label] : this->faceMap)
		{
			db << descriptor << std::endl << label << std::endl;
		}

		this->reporter("The database has been saved.");
	} // try
	catch (const std::ios_base::failure& e)
	{
		throw std::ios_base::failure("Failed to save the database file " + databasePath, e.code());
	}
}	// save




template <class DescriptorComputer, class DescriptorMetric>
std::optional<std::string> FaceDb<DescriptorComputer, DescriptorMetric>::find(const std::string& imageFile, double tolerance) 
{
	std::optional<Descriptor> query = this->descriptorComputer(imageFile);
	if (!query)
		return std::nullopt;

	std::exception_ptr eptr;	// a default-constructed std::exception_ptr is a null pointer; it does not point to an exception object
	std::atomic<bool> eflag{ false };	// exception occurrence flag
	auto best = std::transform_reduce(std::execution::par, this->faceMap.begin(), this->faceMap.end(), 
		std::make_pair(0, std::numeric_limits<double>::infinity()),
		[](const std::pair<std::size_t, double>& x, const std::pair<std::size_t, double>& y) noexcept	// reduce
		{
			return x.second < y.second ? x : y;
		},
		[&query=*query, &eptr, &eflag, this](const std::pair<Descriptor, std::size_t>& p)	// transform
		{
			try
			{
				// TEST!
				// Descriptor metric must not create a race
				//return std::make_pair(p.second, this->descriptorMetric(p.first, query));	// may throw an exception
				return std::make_pair(p.second, DescriptorMetric{ this->descriptorMetric }(p.first, query));	// may throw an exception
				/// TODO: this requires DescriptorMetric to be default-constructible
				//return std::make_pair(p.second, DescriptorMetric{}(p.first, query));	// may throw an exception
			}
			catch (...)
			{
				// Atomically check whether the exception flag has already been set and take care of memory consistency
				if (!eflag.exchange(true, std::memory_order_acq_rel))
					eptr = std::current_exception();

				// std::pair does not throw exceptions unless one of the specified operations (e.g. constructor of an element) throws;
				// std::numeric_limits<double>::infinity() is noexcept.
				return std::make_pair(p.second, std::numeric_limits<double>::infinity());
			}
		});	// transform_reduce

	if (eptr)
		std::rethrow_exception(eptr);

	std::cout << best.second << std::endl;
	//auto test = best.second < tolerance ? this->labels.at(best.first) : "";
	if (best.second < tolerance)
		return this->labels.at(best.first);
	else
		return std::nullopt;
}	// find

