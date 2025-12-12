#pragma once
#include "Scene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "DebugCamera.h"
#include "Player\Player.h"
#include "Particle\P_Fire\P_Fire.h"

class SceneTest2 :public Scene
{
public:
	SceneTest2(kEngine* system);
	~SceneTest2();
	void Update() override;
	void Draw() override;

	void CameraPart();
	void ImguiPart();

private:
	kEngine* system_ = nullptr;				// 借り

	/// ============== カメラ関連 ==============///
	/// ステージカメラ(mainカメラ)
	Camera* camera_ = nullptr;
	/// デバックカメラ
	DebugCamera* debugCamera_ = nullptr;
	/// 今使っているカメラ
	Camera* usingCamera_ = nullptr;			// 借り用

	/// =========== リソースハンドル ============///
	/// モデルハンドル
	int skydomeModelHandle_ = 0;
	int sphereModelHandle_ = 0;

	/// テキスチャーハンドル
	int boxTextureHandle_ = 0;
	int tryTextureHandle_ = 0;

	/// =========== ゲームオブジェクト ==========///
	Object* skydome_ = nullptr;
	Object* plane_[5];
	SimpleSprite* sprite_ = nullptr;
	SimpleSprite* testSprite_[5];



	/// =============== フラグ ===============///
	bool useDebugCamera = false;
};

