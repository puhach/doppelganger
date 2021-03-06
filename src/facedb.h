#ifndef FACEDB_H
#define FACEDB_H

#include <cassert>
#include <vector>	
#include <string>
#include <optional>
#include <functional>
#include <filesystem>

#include <fstream>
#include <iomanip>
#include <execution>
#include <atomic>


/*
* By default L2Distance is used as a measure of similarity between faces (face descriptors). Thus, it must be specialized for
* descriptor types. Alternatively, a custom metric can be defined and specified as a template parameter for FaceDb.
* 
* Specializations of L2Distance and custom metric functors must define a call operator of the signature equivalent to:
*	double operator()(const DescriptorType& descriptor1, const DescriptorType& descriptor2) const
* 
* Additionally, metric functors must be copy-constructible. The call operator may not directly or indirectly modify its arguments
* or any internal data to ensure data-race-free execution in a multithreaded context. 
*/
template <typename T>
struct L2Distance;


/*
* FaceDb creates a database of face descriptors from a directory of input images, stores the descriptors into a file, and provides means
* for loading them in future. It is possible to search for a face in the database using a specified search criterion. 
* 
* This class template is parametrized by a descriptor computer type, which must be a callable object taking in a file or a list of files
* and returning a face descriptor (or a list of optional descriptors) wrapped into std::optional. In case a descriptor cannot be computed 
* for a particular input file, the returned value must be std::nullopt. 
* 
* The descriptor computer type must expose the subtype Descriptor serializable by means of I/O operators. 
* 
* The descriptor metric must be a functor callable in a const context. It has to take two descriptors as arguments and return a double. 
* When two faces represented by descriptors are similar, the returned value should be small. When they are different, the returned value 
* should be high. The call operator must ensure data race avoidance. 
*/

template <class DescriptorComputer, class DescriptorMetric = L2Distance<typename DescriptorComputer::Descriptor>>
class FaceDb final	
{
	using Descriptor = typename DescriptorComputer::Descriptor;
	using Reporter = std::function<void(const std::string&)>;

	static_assert(std::is_invocable_r_v<std::optional<Descriptor>, DescriptorComputer, std::string>
		&& std::is_invocable_r_v<std::vector<std::optional<Descriptor>>, DescriptorComputer, std::vector<std::string>>
		&& std::is_invocable_r_v<std::optional<Descriptor>, DescriptorComputer, std::filesystem::path>
		&& std::is_invocable_r_v<std::vector<std::optional<Descriptor>>, DescriptorComputer, std::vector<std::filesystem::path>>,
		"The descriptor computer must be callable for an input file and a vector of input files. "
		"Return values must be wrapped into std::optional<T>.");
	static_assert(std::is_copy_constructible_v<DescriptorMetric>, "The descriptor metric must be copy-constructible.");
	static_assert(std::is_invocable_r_v<double, const DescriptorMetric&, Descriptor, Descriptor>, 
		"The descriptor metric must be callable in a const context, take two descriptors as arguments, and return a double.");

	// It would also be nice to check whether Descriptor can be serialized/deserialized by means of >> and << operators,
	// but there seems to be no simple way to do it

public:

	FaceDb(const DescriptorComputer& descriptorComputer, DescriptorMetric descriptorMetric = DescriptorMetric()) noexcept(
			std::is_nothrow_copy_constructible_v<DescriptorComputer> &&	std::is_nothrow_copy_constructible_v<DescriptorMetric>)
		: descriptorComputer(descriptorComputer)
		, descriptorMetric(descriptorMetric) {}

	FaceDb(DescriptorComputer&& descriptorComputer, DescriptorMetric descriptorMetric = DescriptorMetric()) noexcept(
			std::is_nothrow_move_constructible_v<DescriptorComputer> && std::is_nothrow_copy_constructible_v<DescriptorMetric>)
		: descriptorComputer(std::move(descriptorComputer))
		, descriptorMetric(descriptorMetric) {}


	FaceDb(const FaceDb& other) = default;
	
	// TODO: test it and probably move to _impl.h file
	FaceDb(FaceDb&& other) = default;
	//FaceDb(FaceDb&& other) noexcept(std::is_nothrow_move_constructible_v<DescriptorComputer> // std::function's move constructor is not noexcept until C++20
	//		&& std::is_nothrow_move_constructible_v<DescriptorMetric> && std::is_nothrow_move_constructible_v<decltype(reporter)>
	//		&& std::is_nothrow_move_constructible_v<decltype(labels)> && std::is_nothrow_move_constructible_v<decltype(facemap)>)	
	//	: descriptorComputer(std::move(other.descriptorComputer))
	//	, descriptorMetric(std::move(const_cast<DescriptorMetric&>(other.descriptorMetric)))
	//	, reporter(std::move(other.reporter))
	//	, labels(std::move(other.labels))
	//	, faceMap(std::move(other.faceMap)) {}

	FaceDb& operator = (const FaceDb& other) = default;		// implicitly deleted due to const metric
	FaceDb& operator = (FaceDb&& other) = default;		// implicitly deleted due to const metric

	//FaceDb& operator = (FaceDb&& other)	noexcept(std::is_nothrow_move_assignable_v<DescriptorComputer>	// std::function's move constructor is not noexcept until C++20
	//	&& std::is_nothrow_move_assignable_v<DescriptorMetric> && std::is_nothrow_move_assignable_v<decltype(reporter)> 
	//	&& std::is_nothrow_move_assignable_v<decltype(labels)> && std::is_nothrow_move_assignable_v<decltype(faceMap)>)
	//{
	//	this->descriptorComputer = std::move(other.descriptorComputer);
	//	this->descriptorMetric = std::move(const_cast<DescriptorMetric&>(other.descriptorMetric));
	//	//this->descriptorMetric = other.descriptorMetric;	// fall back to copying
	//	this->reporter = std::move(other.reporter);
	//	this->labels = std::move(other.labels);
	//	this->faceMap = std::move(other.faceMap);
	//}

	void setReporter(Reporter reporter) { this->reporter = std::move(reporter); }

	void create(const std::string& datasetPath);

	void load(const std::string& databasePath);

	void save(const std::string& databasePath);

	bool enroll(const std::string& imageFile, const std::string& label);

	void clear();

	std::optional<std::string> find(const std::string& filePath, double tolerance);		// non-const since it calls descriptorComputer()
	//std::optional<std::string> find(const std::string& filePath, double tolerance) const;

private:

	DescriptorComputer descriptorComputer;
	const DescriptorMetric descriptorMetric;
	Reporter reporter = [](const std::string& message) noexcept { };
	std::vector<std::string> labels;
	std::vector<std::pair<Descriptor, std::size_t>> faceMap;	
};	// FaceDb




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

			std::fill_n(std::back_inserter(fileLabels), fileEntries.size() - fileLabels.size(), label);

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
			//db >> d >> label;		// descriptors must be deserializable by means of >> operator
			db >> label >> d;		// descriptors must be deserializable by means of >> operator
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
			db << label << std::endl << descriptor << std::endl;
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
	{
		this->reporter("Failed to compute the descriptor for " + imageFile);
		return std::nullopt;
	}

	std::exception_ptr eptr;	// a default-constructed std::exception_ptr is a null pointer; it does not point to an exception object
	std::atomic<bool> eflag{ false };	// exception occurrence flag
	auto best = std::transform_reduce(std::execution::par, this->faceMap.cbegin(), this->faceMap.cend(),
		std::make_pair(0, std::numeric_limits<double>::infinity()),
		[](const std::pair<std::size_t, double>& x, const std::pair<std::size_t, double>& y) noexcept	// reduce
		{
			return x.second < y.second ? x : y;
		},
		[&query = *query, &eptr, &eflag, this](const std::pair<Descriptor, std::size_t>& p)	// transform
		{
			try
			{
				// The descriptor metric must not create a race
				return std::make_pair(p.second, this->descriptorMetric(p.first, query));	// may throw an exception
				//return std::make_pair(p.second, DescriptorMetric{ this->descriptorMetric }(p.first, query));	// may throw an exception
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

	if (best.second < tolerance)
		return this->labels.at(best.first);
	else
		return std::nullopt;
}	// find


template <class DescriptorComputer, class DescriptorMetric>
bool FaceDb<DescriptorComputer, DescriptorMetric>::enroll(const std::string& imageFile, const std::string& label)
{
	std::size_t labelIdx;
	auto it = std::find(this->labels.cbegin(), this->labels.cend(), label);
	if (it == this->labels.end())	// label not found
	{
		this->reporter("Enrolling a new person labeled " + label);
		labelIdx = this->labels.size();
		this->labels.push_back(label);
	}
	else	// this label already exists
	{
		this->reporter("Adding a new face image for " + label);
		labelIdx = it - this->labels.cbegin();
	}

	if (auto descriptor = this->descriptorComputer(imageFile))
	{
		this->faceMap.emplace_back(*std::move(descriptor), labelIdx);
		this->reporter("The descriptor for " + imageFile + " has been added to the database.");
		return true;
	}
	else
	{
		this->reporter("Failed to compute the descriptor for the input file.");
		return false;
	}
}	// enroll

template <class DescriptorComputer, class DescriptorMetric>
void FaceDb<DescriptorComputer, DescriptorMetric>::clear()
{
	this->labels.clear();
	this->faceMap.clear();
	this->reporter("The database has been cleared.");
}	// clear


#endif	// FACEDB_H