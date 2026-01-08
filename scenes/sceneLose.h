#pragma once
#include "Scene.h"
#include "Object\Sprite.h"

class SceneLose :public Scene
{
public:
	SceneLose(kEngine* kEngine);
	~SceneLose();
	void Update() override;
	void Draw() override;

private:

	/// =========== リソースハンドル ============///
	int TH_lose_{};

	int TH_BG_{};
	int TH_pressKey_{};

	int SH_BGM_ = 0;

	/// =========== ゲームオブジェクト ==========///
	SimpleSprite* lose_ = nullptr;
	SimpleSprite* BG_ = nullptr;
	SimpleSprite* pressKey_ = nullptr;

};
