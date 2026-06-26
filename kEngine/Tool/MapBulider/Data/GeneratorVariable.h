#pragma once
#include <vector>
#include <string>
#include <map>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

enum class VarType {
    Bool,
    Int,
    Float,
    Double,
    String,
    Vec2,
    Vec3,
    Vec4
};

struct Variable {
    std::string name;
    VarType type;

    union {
        bool boolValue;
        int intValue;
        float floatValue;
        double doubleValue;
        Vector2 v2Value;
        Vector3 v3Value;
        Vector4 v4Value;
    };

    std::string stringValue; // string 不能放 union
};

struct VariableUnit {
    std::string unitName;
    std::vector<Variable> units;
};

struct GeneratorVariable {
	std::string generatorName;
	std::vector<VariableUnit> unitLists;
	GeneratorVariable();
};