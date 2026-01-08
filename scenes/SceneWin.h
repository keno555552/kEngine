#pragma once
#include "Scene.h"
#include "Object\Sprite.h"

class SceneWin :public Scene
{
public:
	SceneWin(kEngine* kEngine);
	~SceneWin();
	void Update() override;
	void Draw() override;

private:

	/// =========== リソースハンドル ============///
	int TH_win_{};

	int TH_BG_{};
	int TH_pressKey_{};

	int SH_BGM_ = 0;

	/// =========== ゲームオブジェクト ==========///
	SimpleSprite* win_ = nullptr;
	SimpleSprite* BG_ = nullptr;
	SimpleSprite* pressKey_ = nullptr;

};

