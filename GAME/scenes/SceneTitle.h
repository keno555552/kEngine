#pragma once
#include "Object\Object.h"
#include "BaseScene.h"
#include "Object\Sprite.h"
#include "GameObject\Camera\DebugCamera.h"
#include "DefaultMenu\DefaultMenu.h"
#include "GAME\Object\Driller\Driller.h"

class SceneTitle :public BaseScene
{
public:
	SceneTitle(kEngine* system);
	~SceneTitle();

	void Update() override;
	void Draw() override;

	void SetMenu(DefaultMenu* defaultMenu) { defaultMenu_ = defaultMenu; }

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
	std::weak_ptr<Camera> camera_;
	/// デバックカメラ
	std::weak_ptr<DebugCamera> debugCamera_;
	/// 今使っているカメラ
	std::weak_ptr<Camera> usingCamera_;			// 借り用

	DefaultMenu* defaultMenu_ = nullptr;		// 借り用

	/// =========== リソースハンドル ============///
	/// モデルハンドル
	int MH_skydome_{};
	int MH_ground_{};
	int MH_driller_{};

	/// テキスチャーハンドル
	int TH_buleSkySkydome_{};
	int TH_title{};

	int TH_startButton_{};
	int TH_settingButton_{};
	int TH_quitButton_{};
	int TH_buttonBack_notSelect_{};
	int TH_buttonBack_Select_{};
	int TH_background_{};

	/// サウンドハンドル
	int SH_BGM_{};
	int SH_Select_{};
	int SH_Decide_{};

	/// =========== ゲームオブジェクト ==========///
	std::unique_ptr<Object> skydome_;
	std::unique_ptr<Object> ground_;
	std::unique_ptr<Object> background_;
	std::unique_ptr<Driller> driller_;

	SimpleSprite* title_ = new SimpleSprite;

	SimpleSprite* startButton_ = new SimpleSprite;
	SimpleSprite* settingButton_ = new SimpleSprite;
	SimpleSprite* quitButton_ = new SimpleSprite;

	/// =========== background関連 ============///
	float movingSpeed_ = 0.03f;

	/// =============== フラグ ================///
	bool useDebugCamera = false;

	int selectedButtonhandle = 0;

	float buttonStartX_ = 0.0f;

	/// ============= メニュー関連 ============///
	std::function<void()> menuRun[3];

	/// ============= シーン内命令 =============///
	void CameraPart();

#ifdef USE_IMGUI
	void ImGuiPart();
#endif

	void BackgroundMove();
	void UpdateSelect();
	void SelectStart();
	void SelectSetting();
	void SelectQuit();

};