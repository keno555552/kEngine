#pragma once
#include "Utility/EnumNameFinder/EnumNameFinder.h"

enum class DepthStencilType{

	Default = 0,					/// Depth test + write
	ReadOnly,						/// Depth test only
	Disable,						/// Nodepth
	Greater,						/// Reverse-z
	NumberOfDepthStencilStates

};

template <>
inline const std::map<DepthStencilType, std::string> EnumNameMap<DepthStencilType> = {
	{ DepthStencilType::Default,		"Default" },
	{ DepthStencilType::ReadOnly,		"ReadOnly" },
	{ DepthStencilType::Disable,		"Disable" },
	{ DepthStencilType::Greater,		"Greater" }
};
