#pragma once
#include "Utility/EnumNameFinder/EnumNameFinder.h"

enum class BlendModeType
{
	Opaque,
	AlphaBlend,
	AddBlend,
	SubtractBlend,
	MultiplyBlend,
	ScreenBlend,
	NumOfBlends
};

template <>
inline const std::map<BlendModeType, std::string> EnumNameMap<BlendModeType> = {
	{ BlendModeType::Opaque,			"Opaque" },
	{ BlendModeType::AlphaBlend,		"AlphaBlend" },
	{ BlendModeType::AddBlend,			"AddBlend" },
	{ BlendModeType::SubtractBlend,		"SubtractBlend" },
	{ BlendModeType::MultiplyBlend,		"MultiplyBlend" },
	{ BlendModeType::ScreenBlend,		"ScreenBlend" }
};