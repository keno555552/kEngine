#include "SceneManager.h"

SceneManager::SceneManager(kEngine* system) {
	system_ = system;
	sceneUsingHandle_ = SceneNum::S_TITLE;

	InitMaterialConfig(&materialConfig_);
	materialConfig_.uvTransformMatrix =
		MakeAffineMatrix(materialConfig_.uvScale, materialConfig_.uvRotate,
			materialConfig_.uvTranslate);
	materialConfig_.textureHandle =
		system_->LoadTextrue("resources/uvChecker.png");
	// materialConfig_.textureHandle =
	// system_->LoadTextrue("resources/nullScene.png");


	for (auto& ptr : stageIsClear_) {
		ptr = false;
	}
}

SceneManager::~SceneManager() {
	if (sceneUsing_ != nullptr)
		delete sceneUsing_, sceneUsing_ = nullptr;
	if (sceneOld_ != nullptr)
		delete sceneOld_, sceneOld_ = nullptr;
}

void SceneManager::SceneChanger() {

	if (sceneUsing_ != nullptr) {
		if (sceneUsing_->GetScenePhase() == ScenePhase::EXIT) {
			sceneUsingHandle_ = sceneUsing_->GetNextStage();
			delete sceneUsing_, sceneUsing_ = nullptr;
		}
	}


	if (system_->GetTriggerOn(DIK_R) || system_->GetGamepadTriggerOn(VK_PAD_START)) {
		isReset_ = true;
		delete sceneUsing_, sceneUsing_ = nullptr;
	}


	if (sceneUsing_ == nullptr) {


		switch (sceneUsingHandle_) {
		case SceneNum::S_END:
			/// GetIsEndによって外で終わる
			break;

		case SceneNum::S_NONE:
			/// 何もしない, 空番号
			break;

		case SceneNum::S_TESTER:
			sceneUsing_ = new SceneTester(system_);
			// sceneUsing_ = new StageTestForGE(system_);
			break;
		case SceneNum::S_TITLE:

			break;

		case SceneNum::S_SELECT:
			break;
		}
	}
}


void SceneManager::Update() {

	winDataUpdate();
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
			// ImGui::Begin("Position");
			// ImGui::SliderFloat2("LT", &LT.x, 0, 1280.0f);
			// ImGui::SliderFloat2("LB", &LB.x, 0, 1280.0f);
			// ImGui::SliderFloat2("RT", &RT.x, 0, 1280.0f);
			// ImGui::SliderFloat2("RB", &RB.x, 0, 1280.0f);
			// ImGui::End();

			system_->DrawSprite({ 0, 0 }, materialConfig_);
			// system_->DrawSprite({ 0,0 }, materialConfig_, { 64,64 }, { 64,64 * 2 },
			// { 64 * 2,64 }, { 64 * 2,64 * 2 }, 512, 512, { 0,0 }, { 64 * 2,64 * 2
			// }); system_->DrawSprite({ 0,0 }, materialConfig_, LT, LB, RT, RB, 512,
			// 512, { 0,0 }, { 64 * 2,64 * 2 });
		}
	}
}

bool SceneManager::GetIsEnd() {
	if (sceneUsingHandle_ == SceneNum::S_END)return true;
	return false;
}

void SceneManager::winDataUpdate() {

	switch (sceneUsingHandle_) {
	case SceneNum::S_STAGE01:
		if (sceneUsing_->GetIsWin() == IsWin::WIN) stageIsClear_[0] = true;
		break;
	case SceneNum::S_STAGE02:
		if (sceneUsing_->GetIsWin() == IsWin::WIN) stageIsClear_[1] = true;
		break;
	case SceneNum::S_STAGE03:
		if (sceneUsing_->GetIsWin() == IsWin::WIN) stageIsClear_[2] = true;
		break;
	case SceneNum::S_STAGE04:
		if (sceneUsing_->GetIsWin() == IsWin::WIN) stageIsClear_[3] = true;
		break;
	case SceneNum::S_STAGE05:
		if (sceneUsing_->GetIsWin() == IsWin::WIN) stageIsClear_[4] = true;
		break;
	case SceneNum::S_STAGE06:
		if (sceneUsing_->GetIsWin() == IsWin::WIN) stageIsClear_[5] = true;
		break;
	case SceneNum::S_STAGE07:
		if (sceneUsing_->GetIsWin() == IsWin::WIN) stageIsClear_[6] = true;
		break;
	case SceneNum::S_STAGE08:
		if (sceneUsing_->GetIsWin() == IsWin::WIN) stageIsClear_[7] = true;
		break;
	case SceneNum::S_STAGE09:
		if (sceneUsing_->GetIsWin() == IsWin::WIN) stageIsClear_[8] = true;
		break;
	case SceneNum::S_STAGE10:
		if (sceneUsing_->GetIsWin() == IsWin::WIN) stageIsClear_[9] = true;
		break;
	}
}
