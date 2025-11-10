#pragma once
#include <string>

enum class LightModelType {
    NONE = -1,
    Lambert,
    HalfLambert,
    NumLightModels
};

std::string GetLightModelMacro(LightModelType model);