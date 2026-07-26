#pragma once
#include <vector>
#include <string>
#include <map>
#include "Tool\MapBulider\Adapter\ProgramAdapter.h"
#include "Tool\MapBulider\Data\GeneratorVariable.h"
#include "Tool\MapBulider\TileMap.h"
#include "Tool\FileManager\FileManager.h"
#include "Object\Object.h"

struct MapData {
	std::string dataName{};
	int xSize{};
	int ySize{};
	int startX{};
	int startY{};
	std::vector<Object> unitLists{};

	bool isSaved = false;
};

class MapMaker
{
public:
	MapMaker() = default;


	/// ===================== 資料読み取り　===================== ///
	/// 名前関連
	void SetMapName(const std::string& name) { mapName_ = name; }
	std::string GetMapName() const { return mapName_; }
	/// path関連
	void SetProjectPath(const std::string& path) { mapMakerPath_ = path; }
	std::string GetProjectPath() const { return mapMakerPath_; }
	/// Script関連
	void SetCodingText(const std::string& text) { codingText_ = text; }
	const std::string& GetCodingText() const { return codingText_; }
	/// GeneratorVariable取得
	GeneratorVariable& GetGeneratorVariables() { return generatorVariables_; }

	/// ====================== コンパイル　====================== ///
	/// 資料をロジックか、暗号化
	void Compile();
	/// std::stringのJson文章を生成
	void BuildJson();

	/// ======================= 出力関数　======================= ///
	/// MapMaker.json（生成邏輯）を出力 (pathがなければデフォルトパスを使用)
	bool ExportMapMakerToJson(const std::string& outPath = "");
	/// MapData.json（視覺細節）を出力 (pathがなければデフォルトパスを使用)
	bool ExportMapDataToJson(const std::string& outPath = "");


	/// ==================== マップ生成関数　====================== ///
	/// MapMaker内部データに基づいてマップを生成
	TileMapData GenerateMap(MapData& mapData);

	/// MapMaker.json（生成邏輯）を読み込んでマップを生成
	TileMapData GenerateMapFromJson(MapData& mapData, const std::string& jsonPath);


	/// ==================== ローディング関数　==================== ///
	// 載入使用者腳本
	void LoadScript(const std::string& script) { script_ = script; }
	// 取得編譯結果
	const std::string& GetMapMakerJson() const { return mapBuilderJson_; }
	const std::string& GetMapDataJson() const { return mapDataJson_; }

	/// ================== ステータス、エンジン用 ================== ///
	bool isSaved() const { return isSave_; }
	void SetSaved(bool saved) { isSave_ = saved; }

	std::string GetDebugText() const { return debugText_; }

private:
	// 基本資訊
	std::string mapName_;
	std::string mapMakerPath_ = (FileManager::GetDesktopPath());

	// 使用者腳本
	std::string script_;
	std::string codingText_;   // UI 暫存文字

	// 編譯器中間資料
	std::vector<std::string> tokens_;
	std::vector<std::string> sentences_;
	std::vector<Node*> nodes_;

	// 編譯結果 JSON
	std::string mapBuilderJson_;   // MapBuilder.json（生成邏輯）
	std::string mapDataJson_;    // MapDetail.json（視覺細節）

	// 変数
	GeneratorVariable generatorVariables_;

	std::string debugText_; // 編譯器 debug 資訊
	bool isSave_ = false;
private:
	/// emptyならfalseを返す
	std::string CheckSavedPathEmpty(std::string outPath = "");
};

