#pragma once
#include <string>

enum class BlendModeType
{

	NormalBlend,
	AddBlend,
	SubtractBlend,
	MultiplyBlend,
	ScreenBlend,
	NumOfBlends
};

std::string GetBlendModeMarco(BlendModeType model);

