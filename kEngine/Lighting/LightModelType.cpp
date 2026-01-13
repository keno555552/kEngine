#include "LightModelType.h"

std::string GetLightModelMacro(LightModelType model) {
    switch (model) {
    case LightModelType::Lambert:           return "LM_LAMBERT";
    case LightModelType::HalfLambert:       return "LM_HALF";
	case LightModelType::PhongReflection:   return "LM_Phong";
    default:                                return "LM_DEFAULT";
    }
}