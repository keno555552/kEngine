#pragma once
#include <vector>
#include <memory>
#include <map>
#include "externals/nlohmann/json.hpp"
#include "BaseScene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "Lighting\Light.h"
#include "Camera\DebugCamera.h"
#include "GAME\Object\DetailButton\DetailButton.h"
#include "GameObject\button\button.h"
#include "Tool\DefaultMenu\DefaultMenu.h"

#include "Tool\MapBulider\TileMap.h"
#include "Tool\MapBulider\Data\GeneratorVariable.h"
#include "Tool\MapBulider\Adapter\ProgramAdapter.h"
#include "Tool\MapBulider\Data\MapMakerData.h"


enum class SelectModeForMME {
	None,
	TileMap,
	MapMaker,
	Variable,
	MapData,    // 追加
	MapUnit,     // 追加
};

enum class SelectModeForLeftData {
	MapData,
	MapMaker,
	MapList,
};

class MapMakingEditor : public BaseScene
{
public:
	MapMakingEditor(kEngine* system);
	~MapMakingEditor();

	void Update() override;
	void Draw() override;

	bool IsGeneratorMode() const { return modeEditing_ == 2; }
private:

	/// ============ Data関連 =============///
	struct MapInfo {
		TileMapData tileMapData;
		std::string jsonData;
		std::string savePath;
		bool isSaved = false;
		std::string mapDataName;
	};
	std::vector<MapInfo> mapList_{};


	std::vector<MapData> mapDataList_{};

	/// MapData 調整器（View メニューから開閉）
	std::vector<MapMaker> mapMakerlist_{};

	/// ============ 選択関連 =============///

	SelectModeForLeftData selectModeForLeftData_ = SelectModeForLeftData::MapData;
	bool isMapDataSelected_ = false;
	bool isMakerSelected_ = false;
	bool isVariableSelected_ = false;
	int currentMapIndex_ = -1;
	int currentMapDataIndex_ = -1;
	int currentMakerIndex_ = -1;
	int currentVariableIndex_ = -1;
	int currentUnitIndex_ = -1;

	/// 現在のマップ情報
	MapInfo currentMapInfo_{};

	/// ============= Preview関連 ==============///
	std::unique_ptr<Object> previewObject_;          // 選択中の MapUnit を描くための Object
	int previewFocusIndex_ = -2;                     // 前フレームの対象（変わったらカメラを移す）
	int previewMapDataIndex_ = -1;                   // 前フレームの MapData

	/// プレビュー用カメラ（ObjectEditor と同じ自由カメラ）
	Vector3 previewCamPos_{ 0.0f, 1.0f, -4.0f };
	Vector3 previewCamRot_{ 0.245f, 0.0f, 0.0f };    // x = pitch（正で下向き）, y = yaw

	/// ====== MapGenerator 関連
	enum class MapTool { None, Paint, Erase };
	MapTool mapTool_ = MapTool::None;
	int paintUnitId_ = 1;                                   // 塗る物件の代號
	float cellSize_ = 1.0f;                                 // 1マスのワールドサイズ
	int mapResizeBuf_[2] = { 0, 0 };                        // Map Detail のサイズ入力（Apply で反映）

	std::map<int, std::unique_ptr<Object>> mapTileTemplates_;   // 代號 → 見本 Object（モデルが変わった時だけ作り直す）
	std::vector<ObjectData> mapPreviewCells_;               // マスごとの描画データ（毎フレーム作り直す）
	std::vector<ObjectData*> mapPreviewPtrs_;               // ↑のポインタ（DrawEngine に渡す）
	int mapFocusIndex_ = -2;
	Vector3 mapCamPos_{ 0.0f, 0.0f, -20.0f };
	Vector3 mapCamRot_{ 0.0f, 0.0f, 0.0f };


	/// ============ 新規作成関連 =============///
	int newMapCounter_ = 0;
	int newMapDataCounter_ = 0;
	int newMapMakerDataCounter_ = 0;
	int newVariableCounter_ = 0;

	/// 新変数用
	int variableType_ = 0;

	bool newBoolValue_ = false;
	int newIntValue_ = 0;
	float newFloatValue_ = 0.0f;
	double newDoubleValue_ = 0.0;
	std::string newStringValue_ = "";
	Vector2 newVector2Value_ = Vector2(0.0f, 0.0f);
	Vector3 newVector3Value_ = Vector3(0.0f, 0.0f, 0.0f);
	Vector4 newVector4Value_ = Vector4(0.0f, 0.0f, 0.0f, 0.0f);


	/// ============= UI関連 ==============///
	/// ============= 左ウィンドウ
	float menuBarHeight_ = 0.0f;

	float leftInspectorWidth_ = 300.0f;
	int modeEditing_ = 0; // 0 = Terrain, 1 = Room
	bool isMapDataWindowOpen_ = true;
	bool isMapMakerWindowOpen_ = true;
	bool isMapDataListWindowOpen_ = true;
	bool isVariablesWindowOpen_ = true;
	bool showNewVariablesWindow_ = false;

	/// ============= 真ん中ウィンドウ
	bool isMTWindowOpen_ = true;
	bool isMBWindowOpen_ = true;

	/// ============= 右ウィンドウ
	float rightInspectorWidth_ = 300.0f;

	/// ============= 飛び出すウィンドウ
	bool showLoadWindow_ = false;
	bool showSaveWindow_ = false;
	bool showNewWindow_ = false;

	/// ============= Engine Model 選択ウィンドウ ==============///
	bool showEngineModelWindow_ = false;                    // OpenPopup の予約（既存の New 系ウィンドウと同じ方式）
	int engineTargetMapData_ = -1;                          // 開いた時の対象（開いている間に選択が変わっても安全）
	int engineTargetUnit_ = -1;
	EngineModelType engineSelectType_ = EngineModelType::Cube;
	EngineModelParams engineEditParams_{};                  // 入力中の値
	EngineModelParams enginePreviewParams_{};               // 確定した値（プレビュー・作成に使う）
	std::unique_ptr<Object> enginePreviewObject_;           // ウィンドウ内プレビュー用
	std::map<std::string, int> engineModelCache_;           // 作った内蔵モデルの handle（同じ数値で二重に作らない）

	/// 入力データ----------------------(Main!!)
	std::string codeingText_;

	/// セーブパース
	std::string loadFilePath_;
	std::string saveFilePath_;
	std::string projectFilePath_;

	/// NewMapData
	int newMapSize_[2];

	/// DebugData
	std::string debugText_;

	/// 共用
	std::string newName_;
	bool isPause_ = false;
	bool doubleCheck_ = false;

	SelectModeForMME selectMode_ = SelectModeForMME::None;
	bool isSave_ = false;
	bool isLoad_ = false;
	bool isDelete_ = false;

private:
	/// 独立関数
	void DeletePart();
	void MapUpdate(MapData& data, MapMaker& maker);

	/// 頼られる関数
	void MapInfoLoadJson();

	/// プレビュー関連
	void SyncUnitObject(std::unique_ptr<Object>& obj, const MapUnit& unit);
	void SyncUnitObject(const MapUnit& unit) { SyncUnitObject(previewObject_, unit); }
	void FocusPreviewCamera(const MapUnit& unit);
	bool UpdateFreeCamera(Vector3& pos, Vector3& rot, float speed);
	void ImGuiObjectPreview(MapData* data);

	/// MapGenerator 関連
	MapData* FindMapData(const std::string& name);
	void SelectMap(int index);
	void FocusMapCamera(const TileMapData& map);
	void ImGuiMapListWindow(ImVec2 size, ImVec2* outputSize, ImVec2 pos, ImVec2* outputPos, float colimenH);
	void ImGuiMapDetail(MapInfo& info);
	void ImGuiMapPreview();
	void ImGuiMapToolbar();

	/// エンジン内部モデルを呼び出し関連
	int GetEngineModelHandle(EngineModelType type, const EngineModelParams& p);
	void ImGuiEngineModelWindow();

	/// 保存・読み込み関連
	std::string GetModelPath(int modelHandle) const;
	nlohmann::json UnitToJson(const MapUnit& unit) const;
	MapUnit JsonToUnit(const nlohmann::json& j);
	nlohmann::json MapDataToJson(const MapData& data) const;
	MapData JsonToMapData(const nlohmann::json& j);

	bool SaveMapDataFile(MapData& d, const std::string& path);
	bool LoadMapDataFile(const std::string& path, MapData& out);
	MapData* CurrentMapData();
	TileMapData* CurrentMap();
	bool SaveProject(const std::string& projectPath);
	bool LoadProject(const std::string& projectPath);
	void SaveProjectAs();

	/// ImGui関連
	void ImGuiPart();
	void ImGuiLeftMenuBar();
	void ImGuiRightMenuBar();
	void ImGuiMidWindow();

	void ImGuiMapDataWindow(ImVec2 size, ImVec2* outputSize, ImVec2 pos, ImVec2* outputPos, float colimenH);
	void ImGuiMapMakerWindow(ImVec2 size, ImVec2* outputSize, ImVec2 pos, ImVec2* outputPos, float colimenH);

	void ImGuiLoadWindow();
	void ImGuiSaveWindow();
	void ImGuiDoubleCheckWindow();
	void ImGuiNewVariablesWindow();
	void ImGuiMapNewWindow();

	void ImGuiMapUnitDetail(MapData& data, float targetHeight);
	void ImGuiMapDataShowing(MapData& data);
	void ImGuiMapMakerShowing(MapMaker& maker);

	bool InputTextString(const char* label, std::string& str, ImGuiInputTextFlags flags = 0);
	bool InputBigTextString(const char* label, std::string& str, ImGuiInputTextFlags flags = 0);
	void InputCheckbox(const char* label, bool& value);
	void InputInt(const char* label, int& value);
	void InputFloat(const char* label, float& value);
	void InputDouble(const char* label, double& value);
	void InputVector2(const char* label, Vector2& vec);
	void InputVector3(const char* label, Vector3& vec);
	void InputVector4(const char* label, Vector4& vec);
};

