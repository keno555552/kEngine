#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <fstream>
#include "externals/nlohmann/json.hpp"
using std::cout;
using std::endl;

#include <map>
#include "Utility/randomMaker.h"

#include "Tool/MapBulider/TileMap.h"
#include "InstructionCompiler.h"

/// スクリプトの値（int か float）
struct ScriptValue {
	bool isFloat = false;
	int i = 0;
	float f = 0.0f;

	static ScriptValue Int(int v) { ScriptValue s; s.i = v; s.f = static_cast<float>(v); return s; }
	static ScriptValue Float(float v) { ScriptValue s; s.isFloat = true; s.f = v; s.i = static_cast<int>(v); return s; }

	int AsInt() const { return isFloat ? static_cast<int>(f) : i; }      // float → 切り捨て
	float AsFloat() const { return isFloat ? f : static_cast<float>(i); }
	bool AsBool() const { return isFloat ? f != 0.0f : i != 0; }
	/// 型を合わせる（int の変数に 1.7 を入れると 1）
	ScriptValue As(bool toFloat) const { return toFloat ? Float(AsFloat()) : Int(AsInt()); }
};

struct RuntimeContext
{
	int width = 0;
	int height = 0;
	int blankId = 0;

	TileMapData* map = nullptr;
	GeneratorVariable* genVars = nullptr;

	/// スクリプト内の変数（int x; / float f = 0.5; / for の i）
	std::map<std::string, ScriptValue> locals;

	/// 同じ seed → 同じ乱数列
	RandomMaker rng{ true, 0 };

	/// 実行中のエラー（知らない関数・引数の数違い）。同じものは 1 回だけ
	std::vector<std::string> errors;
};

class MapProgramAdapter
{
public:

	/// ===================== 出入力相關 ===================== ///
	static std::string LoadCompile(const std::string& code);
	static void SaveCompile(const std::string& code,
		const std::string& filePath,
		const std::string& name = "");

	/// ==================== TileMap相關 ==================== ///
	static TileMapData LoadMapBulider(const std::string& filePath);
	static TileMapData RunMapBulider(const std::string& filePath,
									 GeneratorVariable* generatorVariables_,
													int blankId = 0,
													int seed = -1);
	static TileMapData Run(const std::vector<Node*>& nodes,
												 int width,
												 int height,
												 int seed,
												 int blankId,
								  GeneratorVariable* vars,
					       std::vector<std::string>* outErrors = nullptr);

	/// ==================== 解碼器（Editor もゲームもここを通る） ===================== ///
/// MapMaker.json の中身から生成（seed < 0 ならファイルの seed、vars が nullptr ならファイルの変数）
	static TileMapData GenerateFromJson(const nlohmann::json& file, 
														  int blankId = 0, 
														  int seed = -1,
									 const GeneratorVariable* vars = nullptr,
									std::vector<std::string>* outErrors = nullptr);
	/// json の "variables" → GeneratorVariable
	static GeneratorVariable JsonToVariables(const nlohmann::json& file);

	/// 組み込み関数を名前で呼ぶ（文・式どちらからも）
	static int CallBuiltin(const std::string& name, const std::vector<ExpressionNode*>& args, RuntimeContext& ctx);

	/// ==================== Ast実行相關 ===================== ///
	static void ExecuteNode(Node* node, RuntimeContext& ctx);
	static ScriptValue EvaluateExpression(ExpressionNode* expr, RuntimeContext& ctx);

	/// ==================== Ast翻訳相關 ===================== ///
	/// AST → JSON
	static nlohmann::json BodyToJson(const std::vector<Node*>& body);
	static nlohmann::json ExpressionToJson(ExpressionNode* expr);

	/// JSON → AST
	static std::vector<Node*> LoadAstFromJson(const nlohmann::json& arr);
	static ExpressionNode* LoadExpressionFromJson(const nlohmann::json& j);
};

