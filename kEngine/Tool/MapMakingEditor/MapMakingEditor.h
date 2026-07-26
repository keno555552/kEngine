#pragma once
#include <vector>
#include <memory>
#include <map>
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
	Variable
};

enum class SelectModeForLeftData {
	MapData,
	MapMaker
};

class MapMakingEditor : public BaseScene
{
public:
	MapMakingEditor(kEngine* system);
	~MapMakingEditor();

	void Update() override;
	void Draw() override;

private:

	/// ============ Data関連 =============///
	struct MapInfo {
		TileMapData tileMapData;
		std::string jsonData;
		std::string savePath;
		bool isSaved;
	};
	std::vector<MapInfo> mapList_{};

	std::vector<MapData> mapDataList_{};
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

	/// 現在のマップ情報
	MapInfo currentMapInfo_{};

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

	/// 入力データ----------------------(Main!!)
	std::string codeingText_;

	/// セーブパース
	std::string loadFilePath_;
	std::string saveFilePath_;

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

	/// ImGui関連
	void ImGuiPart();
	void ImGuiLeftMenuBar();
	void ImGuiRightMenuBar();
	void ImGuiMidWindow();

	void ImGuiMapDataWindow(ImVec2 size,ImVec2* outputSize, ImVec2 pos, ImVec2* outputPos,float colimenH);
	void ImGuiMapMakerWindow(ImVec2 size,ImVec2* outputSize, ImVec2 pos, ImVec2* outputPos, float colimenH);

	void ImGuiLoadWindow();
	void ImGuiSaveWindow();
	void ImGuiDoubleCheckWindow();
	void ImGuiNewVariablesWindow();
	void ImGuiMapNewWindow();

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

