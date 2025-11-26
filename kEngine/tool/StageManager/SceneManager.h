#pragma once
#include "MaterialConfig.h"
#include "Scene.h"
#include "Scene1.h"
#include "SceneTestForGE.h"
#include "SceneTester.h"
#include "SceneTest.h"
#include "SceneTest2.h"

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

	void winDataUpdate();

private:
	Scene* sceneUsing_ = nullptr;
	Scene* sceneOld_ = nullptr;
	MaterialConfig materialConfig_;
	int nullptrSceneHandle_ = 0;

private:
	kEngine* system_ = nullptr; // 借り
	
	bool stageIsClear_[10]{};

	bool isFirst_ = true;

	bool isFromTitle_ = true;

	bool isReset_ = false;

	bool isEnd_ = false;


		// Vector2 LT = {0,0};
		// Vector2 LB = {0,64 };
		// Vector2 RT = {64,0};
		// Vector2 RB = {64,64};
};
