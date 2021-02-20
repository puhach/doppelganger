#ifndef DNNDESCRIPTORCOMPUTER_H
#define DNNDESCRIPTORCOMPUTER_H

#include <optional>
#include <string>




template <class NetworkT, class FaceExtractorT>
class DnnFaceDescriptorComputer
{
	static_assert(std::is_convertible_v<typename FaceExtractorT::Output, typename NetworkT::Input>, "FaceExtractor output type must be compatible with Network input type.");
public:
	using Network = NetworkT;
	using FaceExtractor = FaceExtractorT;
	using Descriptor = typename Network::OutputLabel;

	template <class NetworkT, class FaceExtractorT>
	DnnFaceDescriptorComputer(NetworkT&& network, FaceExtractorT&& faceExtractor) noexcept(
		std::is_nothrow_constructible_v<Network, NetworkT>&& std::is_nothrow_constructible_v<FaceExtractor, FaceExtractorT>)
		: network(std::forward<NetworkT>(network))
		, faceExtractor(std::forward<FaceExtractorT>(faceExtractor))
	{

	}

	// TODO: define copy/move semantics

	template <typename Input>
	//std::optional<typename Network::Descriptor> compute(Input&& input)
	std::optional<Descriptor> operator ()(Input&& input)
	{
		std::optional<typename FaceExtractor::Output> face = faceExtractor(std::forward<Input>(input));
		return face ? network(*face) : std::nullopt;
	}

private:
	FaceExtractor faceExtractor;
	Network network;
};	// DnnFaceDescriptorComputer






#endif	//DNNDESCRIPTORCOMPUTER_H