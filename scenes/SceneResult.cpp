#include "SceneResult.h"

SceneResult::SceneResult(kEngine* system) {
	system_ = system;

	testTextResult = system_->LoadTexture("resources/texture/sceneTest/result.png");

	sprite_->IntObject(system_);
	sprite_->CreateDefaultData();
	sprite_->objectParts_[0].materialConfig->textureHandle = testTextResult;
	sprite_->Update(nullptr);
}

SceneResult::~SceneResult() {
	delete sprite_;
}

void SceneResult::Update() {
	if (system_->GetTriggerOn(DIK_SPACE)) {
		outcome_ = SceneOutcome::NEXT;
	}
}

void SceneResult::Draw() {
	sprite_->Draw();
}
