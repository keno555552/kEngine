#pragma once
#include "Scene.h"
#include "Object\Object.h"
//#include "Camera.h"
#include "DebugCamera.h"

class SceneTest :public Scene
{
public:
	SceneTest(kEngine* system);
	~SceneTest();
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


	/// =========== ゲームオブジェクト ==========///
	Object* player_ = new Object;


	/// =============== フラグ ===============///
	bool useDebugCamera = false;
};

