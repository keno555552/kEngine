#include "GeneratorVariable.h"
#include "Logger.h"

std::string VarTypeToString(VarType type) {
	switch (type) {
	case VarType::Bool: return "Bool";
	case VarType::Int: return "Int";
	case VarType::Float: return "Float";
	case VarType::Double: return "Double";
	case VarType::String: return "String";
	case VarType::Vector2: return "Vector2";
	case VarType::Vector3: return "Vector3";
	case VarType::Vector4: return "Vector4";
	default: return "Unknown";
	}
}

int VarTypeToInt(VarType type) {
	switch (type) {
	case VarType::Bool: return 0;
	case VarType::Int: return 1;
	case VarType::Float: return 2;
	case VarType::Double: return 3;
	case VarType::String: return 4;
	case VarType::Vector2: return 5;
	case VarType::Vector3: return 6;
	case VarType::Vector4: return 7;
	default: return -1;
	}
}

Variable CreateVariable(const std::string& name, const VariantValue value) {
	Variable v;
	v.name = name;
	v.value = value;

	if (std::holds_alternative<bool>(value))            v.type = VarType::Bool;
	else if (std::holds_alternative<int>(value))        v.type = VarType::Int;
	else if (std::holds_alternative<float>(value))      v.type = VarType::Float;
	else if (std::holds_alternative<double>(value))     v.type = VarType::Double;
	else if (std::holds_alternative<std::string>(value))v.type = VarType::String;
	else if (std::holds_alternative<Vector2>(value))    v.type = VarType::Vector2;
	else if (std::holds_alternative<Vector3>(value))    v.type = VarType::Vector3;
	else if (std::holds_alternative<Vector4>(value))    v.type = VarType::Vector4;
	else {
		Logger::Log("Unsupported type in VariantValue");
		return Variable();
	}
	return v;
}

GeneratorVariable::GeneratorVariable() :
	variableList{} {}

Variable* GeneratorVariable::FindVariable(const std::string& name) {
	for (auto& v : variableList) {
		if (v.name == name) return &v;
	}
	return nullptr;
}

std::vector<Variable*> GeneratorVariable::FindTypeVariable(const VarType& type, const std::string& name) {
	std::vector<Variable*> results;
	for (auto& v : variableList) {
		if (v.type == type && v.name == name) {
			results.push_back(&v);
		}
	}
	return results;
}