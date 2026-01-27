#include "SceneTitle.h"

SceneTitle::SceneTitle(kEngine* system) {
	system_ = system;

	//testTextTitle = system_->LoadTexture("resources/texture/sceneTest/title.png");
	testTextTitle = system_->LoadTexture("resources/texture/testBox.png");

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
	ImGuiPart();
}

void SceneTitle::ImGuiPart() {

	ImGui::Begin("Sprite");
	ImGui::SliderFloat3("position", &sprite_->mainPosition.transform.translate.x, -960.0f, 960.0f);
	ImGui::SliderFloat3("rotation", &sprite_->mainPosition.transform.rotate.x, -3.14f, 3.14f);
	ImGui::SliderFloat3("scale", &sprite_->mainPosition.transform.scale.x, 0.0f, 10.0f);
	ImGui::SliderFloat2("LTpos", &sprite_->objectParts_[0].cropLT.x, 1.0f, 400.0f);
	ImGui::SliderFloat2("Size", &sprite_->objectParts_[0].cropSize.x, 1.0f, 400.0f);
	ImGui::End();

}
