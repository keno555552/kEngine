#pragma once
#include <vector>
#include <string>
#include <map>
#include "Tool\MapBulider\Adapter\ProgramAdapter.h"
#include "Tool\MapBulider\Data\GeneratorVariable.h"
#include "Tool\MapBulider\TileMap.h"
#include "Tool\FileManager\FileManager.h"
#include "Object\Object.h"
#include "Utility\EnumNameFinder\EnumNameFinder.h"
#include "Tool\MapBulider\Data\MapDatas.h"

class MapMaker
{
public:
	MapMaker() = default;

	/// ================= 生成設定（MapMaker.json の context） ================= ///
	struct GenerateSettings {
		std::string targetMapName;    // 出力先：空 = Editor 内のマップ、名前あり = Map List のそのマップを上書き
		int width = 32;               // 生成するマップの大きさ（固定。スクリプトがはみ出しても切り捨て）
		int height = 32;
		int seed = 0;                 // 同じ seed → 同じ結果
	};

	/// ===================== 資料読み取り　===================== ///
	/// 名前関連
	void SetMapName(const std::string& name) { mapName_ = name; }
	std::string GetMapName() const { return mapName_; }
	/// path関連
	void SetProjectPath(const std::string& path) { mapMakerPath_ = path; }
	std::string GetProjectPath() const { return mapMakerPath_; }
	/// Script関連
	/// Script関連（今選んでいる函數の腳本）
	void SetCodingText(const std::string& text) { if (MakerFunction* f = CurrentFunction()) f->code = text; }
	const std::string& GetCodingText() const;
	/// GeneratorVariable取得
	GeneratorVariable& GetGeneratorVariables() { return generatorVariables_; }

	/// ====================== コンパイル　====================== ///
	/// std::stringのJson文章を生成
	void BuildJson();

	/// ======================= 出力関数　======================= ///
	/// MapMaker.json（生成邏輯）を出力 (pathがなければデフォルトパスを使用)
	bool ExportMapMakerToJson(const std::string& outPath = "");
	/// MapData.json（視覺細節）を出力 (pathがなければデフォルトパスを使用)
	bool ExportMapDataToJson(const std::string& outPath = "");

	/// ====================== 読み込み関数　====================== ///
	/// MapMaker.json を読み込んで中身を復元（BuildJson の逆）
	bool ImportMapMakerFromJson(const std::string& filePath);

	/// ==================== マップ生成関数　====================== ///
	/// MapMaker内部データに基づいてマップを生成
	TileMapData GenerateMap(int blankId);

	/// MapMaker.json（生成邏輯）を読み込んでマップを生成
	TileMapData GenerateMapFromJson(MapData& mapData, const std::string& jsonPath);

	GenerateSettings& GetGenerateSettings() { return generateSettings_; }
	const GenerateSettings& GetGenerateSettings() const { return generateSettings_; }

	/// ================= 函數（層）：上から順に同じマップへ重ねて実行 ================= ///
	struct MakerFunction {
		std::string name = "Main";
		bool enabled = true;          // false = 生成の時に飛ばす
		std::string code;             // 腳本
	};
	std::vector<MakerFunction>& GetFunctions() { return functions_; }
	int GetCurrentFunction() const { return currentFunction_; }
	MakerFunction* CurrentFunction();
	const MakerFunction* CurrentFunction() const;
	/// 編集する函數を切り替える（範囲外 → -1）
	void SelectFunction(int index);
	/// 追加 / 複製 / 削除 / 並べ替え（選択も追いかける）
	void AddFunction(const std::string& name);
	void DuplicateFunction(int index);
	void RemoveFunction(int index);
	void MoveFunction(int index, int dir);

	/// ==================== ローディング関数　==================== ///
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

	// 函數（層）。最初から 1 つ
	std::vector<MakerFunction> functions_{ MakerFunction{} };
	int currentFunction_ = 0;

	// 編譯結果 JSON
	std::string mapBuilderJson_;   // MapBuilder.json（生成邏輯）
	std::string mapDataJson_;    // MapDetail.json（視覺細節）

	// 変数
	GeneratorVariable generatorVariables_;
	// 生成設定
	GenerateSettings generateSettings_;

	std::string debugText_; // 編譯器 debug 資訊
	bool isSave_ = false;
private:
	/// emptyならfalseを返す
	std::string CheckSavedPathEmpty(std::string outPath = "");
};

