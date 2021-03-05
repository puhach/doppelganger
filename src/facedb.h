#ifndef FACEDB_H
#define FACEDB_H

/// TEST!
//#include <dlib/matrix.h>
//#include "dlibmatrixhash.h"
//#include "myclass.h"

//#include <array>
//#include <fstream>
//#include <filesystem>
//#include <future>
//#include <condition_variable>
#include <cassert>
#include <vector>	
//#include <map>
//#include <unordered_map>
#include <string>
#include <optional>
#include <functional>


// By default L2Distance is used as a measure of similarity between faces (face descriptors). Thus, it must be specialized for
// descriptor types in use. Alternatively, a custom metric can be defined and specified as a template parameter for FaceDb.
template <typename T>
struct L2Distance;


//template <class DescriptorComputer, class DescriptorMetric>
template <class DescriptorComputer, class DescriptorMetric = L2Distance<DescriptorComputer::Descriptor>>
class FaceDb	// TODO: make it final
{
	using Descriptor = typename DescriptorComputer::Descriptor;
	using Reporter = std::function<void(const std::string&)>;

	/*
	//static_assert(std::is_copy_constructible_v<Descriptor>, "The ");
	//static_assert(std::is_default_constructible_v<DescriptorComputer>, "The descriptor computer type must be default-constructible.");
	static_assert(std::is_invocable_r_v<Descriptor, DescriptorComputer, std::string> || 
		std::is_invocable_r_v<std::optional<Descriptor>, DescriptorComputer, std::string>, 
		"The descriptor computer must be invocable with a file path string and return a descriptor or optional<descriptor>.");
	//static_assert(std::is_constructible_v<std::minus<Descriptor>>, "Descriptors must define a distance metric.");
	//static_assert(std::is_invocable_r_v<double, std::minus<Descriptor>, Descriptor, Descriptor>, 
	//	"The distance metric for descriptors must be convertible to double.");	
	static_assert(std::is_default_constructible_v<Descriptor> && std::is_move_constructible_v<Descriptor>, 
		"Descriptors must be default-constructible and move-constructible.");
	static_assert(std::is_move_assignable_v<Descriptor>, "Descriptors must be move-assignable.");
	//static_assert(std::is_default_constructible_v<std::equal_to<Descriptor>>, "Descriptors must be comparable for equality.");
	//static_assert(std::is_default_constructible_v<std::hash<Descriptor>> && std::is_copy_assignable_v<std::hash<Descriptor>> &&
	//	std::is_swappable_v<std::hash<Descriptor>> && std::is_destructible_v<std::hash<Descriptor>>, 
	//	"The standard hash function object for the descriptor type must be defined.");
	static_assert(std::is_default_constructible_v<DescriptorMetric>, "The descriptor metric must be default-constructible.");
	static_assert(std::is_invocable_r_v<double, DescriptorMetric, Descriptor, Descriptor>,
		"The distance metric for descriptors must be defined and convertible to double.");
	*/
	// It would also be nice to check whether Descriptor can be serialized/deserialized by means of >> and << operators,
	// but there seems to be no simple way to do it

public:

	// TODO: noexcept?
	FaceDb(const DescriptorComputer& descriptorComputer, DescriptorMetric descriptorMetric = DescriptorMetric())
		: descriptorComputer(descriptorComputer)
		, descriptorMetric(descriptorMetric) {}

	FaceDb(DescriptorComputer&& descriptorComputer, DescriptorMetric descriptorMetric = DescriptorMetric())
		: descriptorComputer(std::move(descriptorComputer))
		, descriptorMetric(descriptorMetric) {}


	/*FaceDb(Reporter&& reporter = [](const std::string&) {});	
		
	FaceDb(const std::string& database, Reporter&& reporter = [](const std::string&) {});*/

	/*template <typename Reporter = DummyReporter, typename = std::enable_if_t<std::is_invocable_v<Reporter, std::string>>>
	FaceDb(Reporter&& reporter = Reporter())
		: std::forward<Reporter>(reporter) {}

	template <typename Reporter = DummyReporter>
	FaceDb(const std::string& database, Reporter&& reporter = Reporter());*/
	

	// TODO: define copy/move semantics

	void setReporter(Reporter reporter) { this->reporter = std::move(reporter); }

	void create(const std::string& datasetPath);

	void load(const std::string& databasePath);

	void save(const std::string& databasePath);

	//void enroll(const std::string& datasetPath, const std::string& outputPath);

	// TODO: add the clear method

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