#pragma once
#include "MaterialConfig.h"
#include "Scene.h"
#include "Scene1.h"
#include "SceneTestForGE.h"
#include "SceneTester.h"
#include "SceneTest.h"
#include "AnimationSystem/AnimationEditor.h"
#include "DefaultMenu/DefaultMenu.h"

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
	void ImguiPart();
#endif
};
