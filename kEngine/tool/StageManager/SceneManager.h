#pragma once
#include "MaterialConfig.h"
#include "Scene.h"
#include "SceneTester.h"
#include "SceneTest.h"
#include "SceneTest2.h"
#include "CG4_HK_1/Menu.h"
#include "CG4_HK_1/Effect1.h"
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
	
	bool stage[10]{};

	bool isFirst_ = true;

	bool isFromTitle_ = true;

	bool isReset_ = false;

	bool isEnd_ = false;

private:
	/// ============= ステージマネージメント ============///
	void ClearStage();

	void ImguiPart();
};
