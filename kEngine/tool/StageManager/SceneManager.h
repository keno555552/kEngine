#pragma once
#include "MaterialConfig.h"
#include "Scene.h"
#include "SceneTitle.h"
#include "Scene1.h"
#include "SceneResult.h"
#include "SceneTestForGE.h"
#include "SceneTester.h"
#include "SceneTest.h"
#include "SceneTest2.h"
#include "SceneWin.h"
#include "SceneLose.h"
#include "AnimationSystem/AnimationEditor.h"
#include "DefaultMenu/DefaultMenu.h"
#include "CG4_HK_1/Effect2.h"

class SceneManager {
public:
	SceneManager(kEngine* system);
	~SceneManager();

	void Update();
	void Render();

	bool GetIsEnd();

public:
	SceneNum sceneUsingHandle_ = SceneNum::S_NONE;
	void SceneChanger();

	void StageCheckBoxUpdate();


private:
	Scene* sceneUsing_ = nullptr;
	Scene* sceneOld_ = nullptr;

private:
	kEngine* system_ = nullptr; // 借り

	DefaultMenu* defaultMenu_ = nullptr;

	/// ========= リソースハンドル ========= ///

	int helperTextureHandle_ = 0;

	/// ============ オブジェクト =========== ///

	SimpleSprite* helperSprite_ = nullptr;
	Vector2 helperSpriteScale_ = { 1.0f,1.0f };
	Vector2 helperSpritePos_ = { 50.0f,50.0f };

	/// =============== ステージ管理 ===============///
	
	bool stage[10]{};

	bool isFirst_ = true;

	bool isFromTitle_ = true;

	bool isReset_ = false;

	bool isEnd_ = false;

private:
	/// ============= ステージマネージメント ============///
	void ClearStage();

#ifdef USE_IMGUI
	void ImGuiPart();
#endif
};
