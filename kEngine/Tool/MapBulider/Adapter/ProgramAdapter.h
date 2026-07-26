#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <fstream>
#include "externals/nlohmann/json.hpp"
using std::cout;
using std::endl;

#include "Tool/MapBulider/TileMap.h"
#include "InstructionCompiler.h"


struct RuntimeContext
{
	int maxRow = 0;
	int maxCol = 0;
	int seed = 0;

	// for-loop 變數（簡易版本）
	int loopVar = 0;

	TileMapData* map = nullptr;
	GeneratorVariable* genVars;
	// 你之後可以加：
	// float (*noiseFunc)(int, int) = nullptr;
	// RandomGenerator rng;
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
	static TileMapData LoadTileMap(const std::string& filePath);
	static TileMapData RunMapBulider(const std::string& filePath, GeneratorVariable* generatorVariables_);


	/// ==================== Ast実行相關 ===================== ///
	static void ExecuteNode(Node* node, RuntimeContext& ctx);
	static int EvaluateExpression(ExpressionNode* expr, RuntimeContext& ctx);

	/// ==================== Ast翻訳相關 ===================== ///
	/// AST → JSON
	static nlohmann::json BodyToJson(const std::vector<Node*>& body);
	static nlohmann::json ExpressionToJson(ExpressionNode* expr);

	/// JSON → AST
	static std::vector<Node*> LoadAstFromJson(const nlohmann::json& arr);
	static ExpressionNode* LoadExpressionFromJson(const nlohmann::json& j);
};

