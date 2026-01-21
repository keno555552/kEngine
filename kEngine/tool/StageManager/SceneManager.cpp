#include "SceneManager.h"

SceneManager::SceneManager(kEngine* system) {
	system_ = system;
	//sceneUsingHandle_ = SceneNum::S_STAGE_01;
	//sceneUsingHandle_ = SceneNum::S_BOSSTEST;
	sceneUsingHandle_ = SceneNum::S_TESTER;
	//sceneUsingHandle_ = SceneNum::S_TITLE;
	//sceneUsingHandle_ = SceneNum::S_STAGE_01;
	
	

	//InitMaterialConfig(&materialConfig_);
	//materialConfig_.uvTransformMatrix =
	//	MakeAffineMatrix(materialConfig_.uvScale, materialConfig_.uvRotate,
	//		materialConfig_.uvTranslate);
	//materialConfig_.textureHandle =
	//	system_->LoadTexture("resources/TemplateResource/texture/uvChecker.png");
	// materialConfig_.textureHandle =
	// system_->LoadTexture("resources/nullScene.png");

	//for (auto& ptr : stageIsClear_) {
	//	ptr = false;
	//}

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

void SceneManager::SceneChanger() {

	if (sceneUsing_ != nullptr) {
		if (sceneUsing_->GetScenePhase() == ScenePhase::EXIT) {
			sceneUsingHandle_ = sceneUsing_->GetNextStage();
			delete sceneUsing_, sceneUsing_ = nullptr;
		}
	}


	if (sceneUsing_ != nullptr) {
		if (defaultMenu_->IsBack()) {
			sceneUsing_->SetNextStage(SceneNum::S_TITLE);
			sceneUsing_->SetScenePhase(ScenePhase::EXIT);
		}
		if (defaultMenu_->IsRetry()) {
			delete sceneUsing_, sceneUsing_ = nullptr;
		}
	}


	//StageCheckBoxUpdate();

	if (sceneUsing_ == nullptr) {

		if (sceneUsingHandle_ == SceneNum::S_TITLE || sceneUsingHandle_ == SceneNum::S_SELECT) {
			if (defaultMenu_->GetCanOpen()) {
				defaultMenu_->SetCanOpen(false);
			}
		} else {
			if (!defaultMenu_->GetCanOpen()) {
				defaultMenu_->SetCanOpen(true);
			}
		}

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
			//sceneUsing_ = new SceneTest2(system_);
			//sceneUsing_ = new StageTestForGE(system_);
			sceneUsing_ = new Effect2(system_);
			break;

		case SceneNum::S_TITLE:
			sceneUsing_ = new SceneTitle(system_);
			break;

		case SceneNum::S_SELECT:
			break;

		case SceneNum::S_STAGE_01:
			//sceneUsing_ = new SceneTest(system_);
			//sceneUsing_ = new Scene1(system_);
			break;
		case SceneNum::S_STAGE_02:
			break;

		case SceneNum::S_Result:
			sceneUsing_ = new SceneResult(system_);
			break;

		case SceneNum::S_BOSSTEST:
			break;

		case SceneNum::S_ANIMATIONEDITOR:
			sceneUsing_ = new AnimationEditor(system_);
			break;
		}
	}
}


void SceneManager::Update() {


	SceneChanger();

	if (!defaultMenu_->GetIsPause()) {
		if (sceneUsing_ != nullptr) {
			sceneUsing_->Update();
		}
	}

	//helperSprite_->Update(nullptr);

	//defaultMenu_->Update();
}

void SceneManager::Render() {

	if (sceneUsing_ != nullptr) {
		sceneUsing_->Draw();
	} else {
	}

	//helperSprite_->Draw();

	//defaultMenu_->Draw();

#ifdef USE_IMGUI
	ImGuiPart();
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
