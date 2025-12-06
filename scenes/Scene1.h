#pragma once
#include "./kEngine.h"
#include "../StageManager/Scene.h"
#include <memory>
#include <vector>

class Scene1 : public Scene {
public:
	Scene1(kEngine* system);
	~Scene1();

	void Update() override;
	void Draw() override;

private:
	kEngine* system_;
};
