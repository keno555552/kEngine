#include "GeneratorVariable.h"

GeneratorVariable::GeneratorVariable()
    : unitLists{
        {"Bool", {}},
        {"Int", {}},
        {"Float", {}},
		{"Double", {}},
		{"String", {}},
		{"Vector2", {}},
		{"Vector3", {}},
		{"Vector4", {}}
    } {}
