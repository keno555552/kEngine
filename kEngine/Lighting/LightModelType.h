#pragma once
#include <string>

enum class LightModelType {
    NONE = -1,
    Sprite2D,
    Lambert,
    HalfLambert,
    NumLightModels
};

std::string GetLightModelMacro(LightModelType model);