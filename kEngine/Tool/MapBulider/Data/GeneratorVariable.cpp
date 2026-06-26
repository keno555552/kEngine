#include "GeneratorVariable.h"

GeneratorVariable::GeneratorVariable()
    : unitLists{
        {"Bool", {}},
        {"Int", {}},
        {"Float", {}},
		{"Double", {}},
		{"String", {}},
		{"Vec2", {}},
		{"Vec3", {}},
		{"Vec4", {}}
    } {}
