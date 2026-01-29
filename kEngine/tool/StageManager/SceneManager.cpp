#include "SceneManager.h"

SceneManager* SceneManager::sceneManager_ = nullptr;

SceneManager::SceneManager(kEngine* system)
	: system_(system),
	sceneFactory_(new SceneFactory(system)) {
	//sceneUsingHandle_ = SceneNum::S_STAGE_01;
	//sceneUsingHandle_ = SceneNum::S_BOSSTEST;
	sceneUsingNameHandle_ = "TITLE";
	//sceneUsingNameHandle_ = "TITLE";
	//sceneUsingHandle_ = SceneNum::S_TITLE;
	//sceneUsingHandle_ = SceneNum::S_STAGE_01;

	helperTextureHandle_ = system_->LoadTexture("resources/texture/helper.png");
	helperSprite_ = new SimpleSprite;
	helperSprite_->IntObject(system_);
	helperSprite_->CreateDefaultData();
	helperSprite_->mainPosition.transform.scale = { 0.5f,0.5f,1.0f };
	helperSprite_->mainPosition.transform.translate = { 0.0f,550.0f,0.0f };
	helperSprite_->objectParts_[0].materialConfig->textureHandle = helperTextureHandle_;

	defaultMenu_ = new DefaultMenu(system_);
}

SceneManager::~SceneManager() {
	if (sceneUsing_ != nullptr)
		delete sceneUsing_, sceneUsing_ = nullptr;
	if (sceneOld_ != nullptr)
		delete sceneOld_, sceneOld_ = nullptr;
}

void SceneManager::Initialize(kEngine* system) {

	if (!sceneManager_) {
		sceneManager_ = new SceneManager(system);
	}

}

SceneManager& SceneManager::GetInstance() {
	return *sceneManager_;
}

void SceneManager::SceneChanger() {

	if (sceneUsing_) {
		bool isSceneChange = false;

		switch (sceneUsing_->GetOutcome()) {

		case SceneOutcome::NEXT:
		{
			auto targetScene = sceneFlow_.find(sceneUsingNameHandle_);
			if (targetScene != sceneFlow_.end()) {
				sceneUsingNameHandle_ = targetScene->second;
				isSceneChange = true;
			} else {
				Logger::Log("[kError] SM :: SceneChanger: Scene not found in sceneFlow_: " + sceneUsingNameHandle_);
			}
		}
		break;

		case SceneOutcome::RETRY:
			isSceneChange = true;
			break;

		case SceneOutcome::RETURN:
			isSceneChange = true;
			sceneUsingNameHandle_ = "TITLE";
			break;
		case SceneOutcome::EXIT:
			kEngine::EndGame();
			break;

		case SceneOutcome::WIN:
			sceneUsingNameHandle_ = "WIN";
			isSceneChange = true;
			break;
			
		case SceneOutcome::LOSE:
			sceneUsingNameHandle_ = "LOSE";
			isSceneChange = true;
			break;
		}

		if (defaultMenu_->IsBack()) {
			isSceneChange = true;
			sceneUsingNameHandle_ = "TITLE";
		}

		if (defaultMenu_->IsRetry()) {
			isSceneChange = true;
		}

		if (!isSceneChange)return;

		delete sceneUsing_, sceneUsing_ = nullptr;
	}

	sceneUsing_ = sceneFactory_->CreateScene(sceneUsingNameHandle_);
}


void SceneManager::Update() {


	SceneChanger();

	//defaultMenu_->Update();

	if (!defaultMenu_->GetIsPause()) {
		if (sceneUsing_ != nullptr) {
			sceneUsing_->Update();
		}
	}

	//helperSprite_->Update(nullptr);

}

void SceneManager::Render() {

	if (sceneUsing_ != nullptr) {
		sceneUsing_->Draw();
	} else {
	}

	//helperSprite_->Draw();

	defaultMenu_->Draw();

#ifdef USE_IMGUI
	ImGuiPart();
#endif
}

//void SceneManager::StageCheckBoxUpdate() {
//	int checker = 0;
//	int newStageNum = -1;
//
//	for (int i = 0; i < 10; ++i) {
//		if (stage[i]) {
//			checker++;
//			if (i != static_cast<int>(sceneUsingHandle_)) {
//				newStageNum = i;
//			}
//		}
//	}
//
//	if (checker > 1 && newStageNum != -1) {
//		ClearStage();
//		stage[newStageNum] = true;
//		sceneUsingHandle_ = static_cast<SceneNum>(newStageNum);
//	}
//
//	if (checker == 0) {
//		ClearStage();
//		stage[0] = true;
//		sceneUsingHandle_ = SceneNum::S_SELECT;
//	}
//}


void SceneManager::ClearStage() {
	for (auto& ptr : stage) {
		ptr = false;
	}
	delete sceneUsing_, sceneUsing_ = nullptr;
}

#ifdef USE_IMGUI
void SceneManager::ImGuiPart() {
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
		ImGui::Begin("MenuTest");
		if (defaultMenu_->isClicked()) {
			ImGui::Text("IsClicked: True");
		} else {
			ImGui::Text("IsClicked: False");
		}

		if (defaultMenu_->IsRetry()) {
			ImGui::Text("IsRetry: True");
		} else {
			ImGui::Text("IsRetry: False");
		}

		if (defaultMenu_->IsBack()) {
			ImGui::Text("IsBack: True");
		} else {
			ImGui::Text("IsBack: False");
		}
		ImGui::End();
	}
}
#endif
