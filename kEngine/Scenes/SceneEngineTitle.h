#pragma once
#include "Object\Object.h"
#include "BaseScene.h"
#include "Object\Sprite.h"
#include "GameObject\Camera\DebugCamera.h"
#include "DefaultMenu\DefaultMenu.h"
#include "Tool\MovementObject\MovementObject.h"

class SceneEngineTitle :public BaseScene
{
public:
	SceneEngineTitle(kEngine* system);
	~SceneEngineTitle();

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

	/// ============== 後処理関連 ==============///
	RenderCommand colorGradingCommand_;
	int ColorGradingPassHandle_ = -1;

	/// ============== カメラ関連 ==============///

	DefaultMenu* defaultMenu_ = nullptr;		// 借り用

	/// =========== リソースハンドル ============///
	/// テキスチャーハンドル
	int TH_title{};

	/// サウンドハンドル
	int SH_BGM_{};
	int SH_Decide_{};

	/// =========== ゲームオブジェクト ==========///
	std::unique_ptr<SimpleSprite> title_;
	std::unique_ptr<SimpleSprite> BG_;

	std::unique_ptr<MovementObject> sharkObject_;
	/// ============= メニュー関連 ============///
	Vector4 BGColor_ = {0.0f, 0.0f, 0.0f, 1.0f};

	/// ========== シーン切り替え関連 ==========///
	float entryTime_ = 2.0f;
	float showTime_ = 5.0f;

	bool isskip_ = false;

private:

#ifdef USE_IMGUI
	void ImGuiPart();
#endif

	void PhaseUpdate();


};