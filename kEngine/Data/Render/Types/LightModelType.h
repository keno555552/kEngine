#pragma once
#include <string>

enum class LightModelType {
    NONE = -1,
    Sprite2D,
    Lambert,
    HalfLambert,
    PhongReflection,
    BlinnPhongReflection,
	FlameNeonGlow,
    DebugLine,
	SkyCube,
    NumLightModels,
};

std::string GetLightModelMacro(LightModelType model);