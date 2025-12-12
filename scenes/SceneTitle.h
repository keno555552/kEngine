#pragma once
#include "Scene.h"
#include "Object\Sprite.h"
#include "DebugCamera.h"

class SceneTitle :public Scene
{
public:
	SceneTitle(kEngine* system);
	~SceneTitle();

	void Update() override;
	void Draw() override;

public:
	/// ============== カメラ関連 ==============///
	/// ステージカメラ(mainカメラ)
	Camera* camera_ = nullptr;


	/// ========= テキスチャーハンドル =========///
	int testTextTitle{};

	/// ============ オブジェクト ============///
	SimpleSprite* sprite_ = new SimpleSprite;

};