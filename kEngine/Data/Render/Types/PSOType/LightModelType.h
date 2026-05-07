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
    //BlinnPhongReflection_Skinning,
    NumLightModels,
};

//enum class LightModelType {
//    NONE = -1,
//    Lambert,
//    HalfLambert,
//    PhongReflection,
//    BlinnPhongReflection,
//    FlameNeonGlow,
//    DebugLine,
//    SkyCube,
//    //BlinnPhongReflection_Skinning,
//    NumLightModels,
//};


std::string GetLightModelMacro(LightModelType model);