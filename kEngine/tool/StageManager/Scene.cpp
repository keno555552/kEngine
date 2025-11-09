#include "Scene.h"


void Scene::Update() {
}

void Scene::Draw() {
}

void Scene::ChangeNextStage(SceneNum sceneNum) {
	nextStage = sceneNum;
	phase_ = ScenePhase::EXIT;
}

void Scene::EndGame() {
	nextStage = SceneNum::S_END;
	phase_ = ScenePhase::EXIT;
}
