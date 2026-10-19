#pragma once
#include <string>
#include "Utility/EnumNameFinder/EnumNameFinder.h"

enum class LightModelType {
    NONE = -1,
    Sprite2D,
    Lambert,
    HalfLambert,
    PhongReflection,
    BlinnPhongReflection,
	FlameNeonGlow,
    DebugLine,
	Environment,
    NumLightModels,
};

template <>
inline const std::map<LightModelType, std::string> EnumNameMap <LightModelType> = {
	{ LightModelType::NONE,						"NONE" },
	{ LightModelType::Sprite2D,					"Sprite2D" },
	{ LightModelType::Lambert,					"Lambert" },
	{ LightModelType::HalfLambert,				"HalfLambert" },
	{ LightModelType::PhongReflection,			"PhongReflection" },
	{ LightModelType::BlinnPhongReflection,		"BlinnPhongReflection" },
	{ LightModelType::FlameNeonGlow,			"FlameNeonGlow" },
	{ LightModelType::DebugLine,				"DebugLine" },
	{ LightModelType::Environment,				"Environment" }
};

/// エンジン内部で使用する名前を取得する
std::string GetLightModelMacro(LightModelType model);