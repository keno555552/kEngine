#pragma once
#include "Scene.h"
#include "MathsIncluder.h"
#include "config.h"

class SceneTestForGE :public Scene
{
public:
	SceneTestForGE(kEngine* system);
	~SceneTestForGE();
	void Update() override;
	void Draw() override;

private:
	kEngine* system_ = nullptr;


};

