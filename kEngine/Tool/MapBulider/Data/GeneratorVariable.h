#pragma once
#include <vector>
#include <string>
#include <map>
#include <variant>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
/// 便利のため、支援の形を先に定義しとく
using VariantValue = std::variant<
	bool,
	int,
	float,
	double,
	std::string,
	Vector2,
	Vector3,
	Vector4
>;

/// 変数の型を列挙するためのenum
enum class VarType {
	Bool,
	Int,
	Float,
	Double,
	String,
	Vector2,
	Vector3,
	Vector4
};

/// enum翻訳用
std::string VarTypeToString(VarType type);
int VarTypeToInt(VarType type);

/// 実際一つの変数の構造体
struct Variable {
	std::string name;
	VarType type;
	VariantValue value;
};

/// 安全のために作るCreate関数
Variable CreateVariable(const std::string& name, const VariantValue value);

/// 便利のためのGet関数、じゃないと毎回std::get<type>を書かなといといけない
template<typename T>
T GetValue(const Variable& var) {
	return std::get<T>(var.value);
}

/// 書くこともできるGet関数
template<typename T>
T& GetValueRef(Variable& var) {
	return std::get<T>(var.value);
}

/// 生成器の変数をまとめる構造体
struct GeneratorVariable {
	std::string generatorName;
	std::vector<Variable> variableList;
	GeneratorVariable();
	Variable* FindVariable(const std::string& name);
	std::vector<Variable*> FindTypeVariable(const VarType& type, const std::string& name);
};

/// std::variantはC++17以降で導入された型安全な共用体（union）のようなもので、複数の型のいずれかを保持できるデータ構造です。
/// std::holds_alternativeを使って形を確認し、std::getを使って値を取得することができます。
/// ここのtypeは方便のために、建造の時と共に保持しておくためのものです。
