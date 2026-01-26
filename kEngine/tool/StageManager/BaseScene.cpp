#include "BaseScene.h"


void BaseScene::Update() {
}

void BaseScene::Draw() {
}

void BaseScene::ChangeNextStage(SceneNum sceneNum) {
	nextStage = sceneNum;
	phase_ = ScenePhase::EXIT;
}

void BaseScene::EndGame() {
	nextStage = SceneNum::S_END;
	phase_ = ScenePhase::EXIT;
}
