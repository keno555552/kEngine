#include "SceneManager.h"

SceneManager::SceneManager(kEngine* system) {
	system_ = system;
	//sceneUsingHandle_ = SceneNum::S_SELECT;
	sceneUsingHandle_ = SceneNum::S_EFFECT1;

	for (int i = 0; i < 10; ++i) {
		stage[i] = false;
		if (i == static_cast<int>(sceneUsingHandle_)) {
			stage[i] = true;
		}
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


	//if (system_->GetTriggerOn(DIK_R) || system_->GetGamepadTriggerOn(VK_PAD_START)) {
	//	isReset_ = true;
	//	delete sceneUsing_, sceneUsing_ = nullptr;
	//}

	StageCheckBoxUpdate();

	if (sceneUsing_ == nullptr) {

		switch (sceneUsingHandle_) {
		case SceneNum::S_END:
			/// GetIsEndによって外で終わる
			break;

		case SceneNum::S_NONE:
			/// 何もしない, 空番号
			break;

		case SceneNum::S_TESTER:
			//sceneUsing_ = new SceneTester(system_);
			//sceneUsing_ = new SceneTest(system_);
			sceneUsing_ = new SceneTest2(system_);
			// sceneUsing_ = new StageTestForGE(system_);
			break;
		case SceneNum::S_SELECT:
			//sceneUsing_ = new Menu(system_);
			break;
		case SceneNum::S_EFFECT1:
			sceneUsing_ = new Effect1(system_);
			break;
		case SceneNum::S_EFFECT2:
			sceneUsing_ = new Effect2(system_);
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
	}
#ifdef USE_IMGUI
	ImguiPart();
#endif
}

bool SceneManager::GetIsEnd() {
	if (sceneUsingHandle_ == SceneNum::S_END)return true;
	return false;
}

void SceneManager::StageCheckBoxUpdate() {
	int checker = 0;
	int newStageNum = -1;

	for (int i = 0; i < 10; ++i) {
		if (stage[i]) {
			checker++;
			if (i != static_cast<int>(sceneUsingHandle_)) {
				newStageNum = i;
			}
		}
	}

	if (checker > 1 && newStageNum != -1) {
		ClearStage();
		stage[newStageNum] = true;
		sceneUsingHandle_ = static_cast<SceneNum>(newStageNum);
	}

	if (checker == 0) {
		ClearStage();
		stage[0] = true;
		sceneUsingHandle_ = SceneNum::S_SELECT;
	}
}


void SceneManager::ClearStage() {
	for (auto& ptr : stage) {
		ptr = false;
	}
	delete sceneUsing_, sceneUsing_ = nullptr;
}

#ifdef USE_IMGUI
void SceneManager::ImguiPart() {
	{
		float fps = system_->GetFPS();
		float fps1s = system_->GetFPSPerSecond();
		float deltaTime = system_->GetDeltaTime();
		ImGui::Begin("FPS");
		ImGui::InputFloat("FPS", &fps);
		ImGui::InputFloat("FPS_1s", &fps1s);
		ImGui::InputFloat("deltaTime", &deltaTime);
		ImGui::End();
	}

	{
		ImGui::Begin("Stage");

		ImGui::Checkbox("Menu", &stage[0]);
		std::string stageName[3];
		for (int i = 1; i < 3; i++) {
			stageName[i] = "Effect_" + std::to_string(i);
			ImGui::Checkbox(stageName[i].c_str(), &stage[i]);
		}


		ImGui::End();
	}
}
#endif
