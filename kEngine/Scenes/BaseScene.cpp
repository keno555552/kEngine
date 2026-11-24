#include "BaseScene.h"


void BaseScene::Update() {}

void BaseScene::Draw() {}

void BaseScene::Pause() {}

void BaseScene::Resume() {}

void BaseScene::EndGame() {
	outcome_ = SceneOutcome::EXIT;
}
