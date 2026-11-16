#pragma once
#include "BaseScene.h"
#include "Object\Object.h"
#include "Tool\MapBulider\MapChipManager.h"
#include "Tool\MapBulider\Data\MapBlockData.h"
#include "Tool\MapBulider\Data\MapMakerStore.h"
#include <memory>
#include <set>
#include <string>

/// マップ読み込み・生成のテスト用シーン（debugCamera + ブロックだけ）
///  - Load CSV ：csv をそのまま読む（今のゲームと同じ）
///  - Generate ：mmproj の MapMaker で生成 → 同じ流れでレイヤーに入れる
class SceneMapTest : public BaseScene {
public:
	SceneMapTest(kEngine* system);
	~SceneMapTest();
	void Update() override;
	void Draw() override;

private:
	/// ========== 読み込み・生成 ========== ///
	bool LoadCsv();                        // csv → レイヤー
	bool LoadGenerated();                  // MapMaker → TileMapData → レイヤー
	void BuildBlocks();                    // レイヤー → ブロック（Object）
	void SpawnDefault(const MapReader::SpawnInfo& s);
	void ImGuiPart();

private:
	kEngine* system_ = nullptr;            // 借り
	std::weak_ptr<DebugCamera> debugCamera_;

	std::unique_ptr<MapChipManager> mapChipManager_;
	MapBlockData mapBlocks_;
	MapMakerStore makerStore_;

	/// ========== パス（tutorial のデータ） ========== ///
	std::string csvPath_ = "GAME/resources/Stage/1YearGame/Maps/tutorial.csv";
	std::string mapDataPath_ = "GAME/resources/Stage/1YearGame/MapData/tutorial.json";
	std::string mmprojPath_ = "GAME/resources/Stage/1YearGame/1YearGame.mmproj";
	const std::string kLayer_ = "Main";

	/// ========== 状態 ========== ///
	enum class Request { None, Csv, Generate };
	Request request_ = Request::Csv;       // 最初は csv を読む（Update の頭で処理）
	int makerHandle_ = -1;                 // 生成に使う MapMaker
	int seed_ = -1;                        // -1 = MapMaker の seed
	float drawRadius_ = 60.0f;

	std::string log_;                      // ImGui に出す結果
	std::set<std::string> unhandled_;      // 生成されなかった物（Player など）
};