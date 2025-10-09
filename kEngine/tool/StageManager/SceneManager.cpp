#include "SceneManager.h"


SceneManager::SceneManager(kEngine* system) {
	system_ = system;
	sceneUsingHandle_ = SceneNum::STAGE;

	intMaterialConfig(&materialConfig_);
	materialConfig_.uvTransformMatrix = MakeAffineMatrix(materialConfig_.uvScale, materialConfig_.uvRotate, materialConfig_.uvTranslate);
	materialConfig_.textureHandle = system_->LoadTextrue("resources/uvChecker.png");
	//materialConfig_.textureHandle = system_->LoadTextrue("resources/nullScene.png");

}

SceneManager::~SceneManager() {
	if (sceneUsing_ != nullptr) delete sceneUsing_, sceneUsing_ = nullptr;
	if (sceneOld_ != nullptr) delete sceneOld_, sceneOld_ = nullptr;
}

void SceneManager::SceneChanger() {

	if (system_->GetTriggerOn(DIK_TAB)) {
		delete sceneUsing_, sceneUsing_ = nullptr;
	}


	if (sceneUsing_ == nullptr) {
		switch (sceneUsingHandle_) {
		case SceneNum::NONE:
			break;
		case SceneNum::TESTER:
			sceneUsing_ = new SceneTester(system_);
			//sceneUsing_ = new StageTestForGE(system_);
			break;
		case SceneNum::TITLE:
			break;
		case SceneNum::STAGE:
			sceneUsing_ = new Scene1(system_);
			break;
		case SceneNum::WIN:
			break;
		case SceneNum::GAMEOVER:
			break;
		}
	}
}

void SceneManager::Update() {
	SceneChanger();
	if (sceneUsing_ != nullptr) {
		sceneUsing_->Update();
	}

}


void SceneManager::Render() {
	if (sceneUsing_ != nullptr) {
		sceneUsing_->Draw();
	} else {
		if (sceneOld_ == nullptr) {
			//ImGui::Begin("Position");
			//ImGui::SliderFloat2("LT", &LT.x, 0, 1280.0f);
			//ImGui::SliderFloat2("LB", &LB.x, 0, 1280.0f);
			//ImGui::SliderFloat2("RT", &RT.x, 0, 1280.0f);
			//ImGui::SliderFloat2("RB", &RB.x, 0, 1280.0f);
			//ImGui::End();

			system_->DrawSprite({ 0,0 }, materialConfig_);
			//system_->DrawSprite({ 0,0 }, materialConfig_, { 64,64 }, { 64,64 * 2 }, { 64 * 2,64 }, { 64 * 2,64 * 2 }, 512, 512, { 0,0 }, { 64 * 2,64 * 2 });
			//system_->DrawSprite({ 0,0 }, materialConfig_, LT, LB, RT, RB, 512, 512, { 0,0 }, { 64 * 2,64 * 2 });
		}
	}
}