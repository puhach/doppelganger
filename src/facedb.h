#ifndef FACEDB_H
#define FACEDB_H

#include <cassert>
#include <vector>	
#include <string>
#include <optional>
#include <functional>
#include <filesystem>

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
* The descriptor metric must be a functor callable in a const context, take two descriptors as arguments, and return a double. When two
* faces represented by descriptors are similar, the returned value should be small. When they are different, the returned value should be 
* high. The call operator must ensure data race avoidance. 
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
	Reporter reporter = [](const std::string& message) { };
	//std::unordered_map<Descriptor, std::size_t> faceMap;
	std::vector<std::string> labels;
	std::vector<std::pair<Descriptor, std::size_t>> faceMap;	
};	// FaceDb

#include "facedb_impl.h"

#endif	// FACEDB_H