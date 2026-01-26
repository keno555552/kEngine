#pragma once
#include "BaseScene.h"
#include "MathsIncluder.h"
#include "config.h"

class SceneTestForGE :public BaseScene
{
public:
	SceneTestForGE(kEngine* system);
	~SceneTestForGE();
	void Update() override;
	void Draw() override;

private:
	kEngine* system_ = nullptr;


};

