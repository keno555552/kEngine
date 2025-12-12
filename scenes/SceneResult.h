#pragma once
#include "Scene.h"
#include "Object\Sprite.h"

class SceneResult :public Scene
{
public:
	SceneResult(kEngine* system);
	~SceneResult();

	void Update() override;
	void Draw() override;

public:

	/// ========= テキスチャーハンドル =========///
	int testTextResult{};

	/// ============ オブジェクト ============///
	SimpleSprite* sprite_ = new SimpleSprite;
};

