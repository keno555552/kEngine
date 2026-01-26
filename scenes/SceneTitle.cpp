#include "SceneTitle.h"

SceneTitle::SceneTitle(kEngine* system) {
	system_ = system;

	testTextTitle = system_->LoadTexture("resources/texture/sceneTest/title.png");

	sprite_->IntObject(system_);
	sprite_->CreateDefaultData();
	sprite_->objectParts_[0].materialConfig->textureHandle = testTextTitle;
	sprite_->Update(nullptr);

}

SceneTitle::~SceneTitle() {
	delete sprite_;
}

void SceneTitle::Update() {

	if (system_->GetTriggerOn(DIK_SPACE)) {
		outcome_ = SceneOutcome::NEXT;
	}
}

void SceneTitle::Draw() {
	sprite_->Draw();
}
