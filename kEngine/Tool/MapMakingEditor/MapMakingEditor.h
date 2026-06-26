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
		std::string savePath;
		bool isSaved;
	};
	std::vector<std::unique_ptr<MapInfo>> mapData_{};
	int selectedMap_ = -1;
	int newCounter_ = 0;



	/// ============= UI関連 ==============///
	/// ============= 左ウィンドウ
	float menuBarHeight_ = 0.0f;

	float leftInspectorWidth_ = 300.0f;
	int modeEditing_ = 0; // 0 = Terrain, 1 = Room
	bool isMapWindowOpen_ = true;
	bool isGenratorWindowOpen_ = true;
	bool isVariablesWindowOpen_ = true;

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
	std::string newMapName_;

private:
	void ImGuiPart();
	void ImGuiLeftMenuBar();
	void ImGuiRightMenuBar();
	void ImGuiMidWindow();
	void ImGuiLoadWindow();
	void ImGuiSaveWindow();
	void ImGuiNewWindow();
	bool InputTextString(const char* label, std::string& str, ImGuiInputTextFlags flags = 0);
	bool InputBigTextString(const char* label, std::string& str, ImGuiInputTextFlags flags = 0);

};

