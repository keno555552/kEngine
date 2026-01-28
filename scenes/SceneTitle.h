#pragma once
#include "Object\Object.h"
#include "BaseScene.h"
#include "Object\Sprite.h"
#include "DebugCamera.h"
#include "DefaultMenu/DefaultMenu.h"

class SceneTitle :public BaseScene
{
public:
	SceneTitle(kEngine* system);
	~SceneTitle();

	void Update() override;
	void Draw() override;

	void GetMenu(DefaultMenu* defaultMenu) { defaultMenu_ = defaultMenu; }

private:
	enum TitleButton {
		START,
		SETTING,
		QUIT,
		MAX
	};

private:
	/// ============== カメラ関連 ==============///
	/// ステージカメラ(mainカメラ)
	Camera* camera_ = nullptr;
	/// デバックカメラ
	DebugCamera* debugCamera_ = nullptr;
	/// 今使っているカメラ
	Camera* usingCamera_ = nullptr;			// 借り用

	DefaultMenu* defaultMenu_ = nullptr;		// 借り用

	/// =========== リソースハンドル ============///
	/// モデルハンドル
	int MH_skydome_{};
	int MH_ground_{};

	/// テキスチャーハンドル
	int TH_buleSkySkydome_{};
	int TH_title{};

	int TH_startButton_{};
	int TH_settingButton_{};
	int TH_quitButton_{};
	int TH_buttonBack_notSelect_{};
	int TH_buttonBack_Select_{};

	int SH_BGM_{};
	int SH_Select_{};
	int SH_Decide_{};

	/// =========== ゲームオブジェクト ==========///
	Object* skydome_ = nullptr;
	Object* ground_ = nullptr;

	SimpleSprite* title_ = new SimpleSprite;

	SimpleSprite* startButton_ = new SimpleSprite;
	SimpleSprite* settingButton_ = new SimpleSprite;
	SimpleSprite* quitButton_ = new SimpleSprite;

	/// =============== フラグ ================///
	bool useDebugCamera = false;

	int selectedButtonhandle = 0;

	/// ============= メニュー関連 ============///
	std::function<void()> menuRun[3];

	/// ============= シーン内命令 =============///
	void CameraPart();

#ifdef USE_IMGUI
	void ImGuiPart();
#endif

	void UpdateSelect();
	void SelectStart();
	void SelectSetting();
	void SelectQuit();

private:
	void ImGuiPart();

};