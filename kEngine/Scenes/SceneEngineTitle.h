#pragma once
#include "Object\Object.h"
#include "BaseScene.h"
#include "Object\Sprite.h"
#include "GameObject\Camera\DebugCamera.h"
#include "DefaultMenu\DefaultMenu.h"
#include "GAME\Object\Driller\Driller.h"

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

	/// ============= メニュー関連 ============///
	Vector4 BGColor_ = {0.0f, 0.0f, 0.0f, 1.0f};

#ifdef USE_IMGUI
	void ImGuiPart();
#endif
};