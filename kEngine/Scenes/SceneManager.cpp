#include "SceneManager.h"

std::unique_ptr <SceneManager> SceneManager::sceneManager_ = nullptr;

void SceneManager::Initialize(kEngine* system) {
	system_ = system;
	sceneFactory_ = std::make_unique<SceneFactory>(system);
	sceneUsingNameHandle_ = "ENGINE_TITLE";
	//sceneUsingNameHandle_ = "TITLE";
	//sceneUsingNameHandle_ = "STAGE_TUTORIAL";
	//sceneUsingNameHandle_ = "CGHK2";
	//sceneUsingNameHandle_ = "UITest";
	//sceneUsingNameHandle_ = "ParticleEditor";
	//sceneUsingNameHandle_ = "EffectEditor";
	//sceneUsingNameHandle_ = "ANIMATIONEDITOR";

	/// チートエンジンの初期化
#ifdef _DEBUG
	cheatEngine_ = std::make_unique<CheatEngine>();
	cheatEngine_->Initialize(system_);
#endif

	helperTextureHandle_ = system_->LoadTexture("./kEngine/EngineAssets/texture/helper.png");
	helperSprite_ = std::make_unique <SimpleSprite>();
	helperSprite_->IntObject(system_);
	helperSprite_->CreateDefaultData();
	helperSprite_->mainPosition.transform.scale = { 0.5f,0.5f,1.0f };
	helperSprite_->mainPosition.transform.translate = { 0.0f,550.0f,0.0f };
	helperSprite_->objectParts_[0].materialConfig->textureHandle = helperTextureHandle_;

	config::default_White5x5_TextureHandle_ = system_->LoadTexture("./kEngine/EngineAssets/TemplateResource/texture/white5x5.png");

	defaultMenu_ = std::make_unique <DefaultMenu>(system_);
}

void SceneManager::Finalize() {

#ifdef _DEBUG
	cheatEngine_.reset();
#endif

	sceneUsing_.reset();
	sceneOld_.reset();
	sceneFactory_.reset();
	defaultMenu_.reset();
	helperSprite_.reset();
}

SceneManager& SceneManager::GetInstance() {
	if (!sceneManager_) {
		sceneManager_ = std::make_unique<SceneManager>(ConstructorKey());
	}
	return *sceneManager_;
}

void SceneManager::SceneChanger() {

	if (sceneUsing_) {

		/// ステージが変わるかどうかのフラグ
		bool isSceneChange = false;

		/// ① デフォルトメニュー（シーンの結果より優先。結果が NONE でも効く）
		if (defaultMenu_) {
			if (defaultMenu_->IsBack()) {
				sceneUsingNameHandle_ = "TITLE";
				isSceneChange = true;
			} else if (defaultMenu_->IsRetry()) {
				isSceneChange = true;          // 同じシーンを作り直す
			}
		}

		/// ② シーンの結果
		if (!isSceneChange) {
			SceneOutcome outcome = sceneUsing_->GetOutcome();

			/// 戻りがない場合シーン転移しない
			if (outcome == SceneOutcome::NONE) return;

			/// EXITが押されたらゲーム終了
			if (outcome == SceneOutcome::EXIT) {
				kEngine::EndGame();
				return;
			}

			auto targetFlow = sceneFlow_.find(sceneUsingNameHandle_);
			if (targetFlow == sceneFlow_.end()) {
				Logger::Log("[kError] SM :: SceneChanger: Scene not found in sceneFlow_: " + sceneUsingNameHandle_);
			} else {
				auto targetScene = targetFlow->second.find(outcome);
				if (targetScene == targetFlow->second.end()) {
					Logger::Log("[kError] SM :: SceneChanger: Outcome not found in sceneFlow_: " + sceneUsingNameHandle_);
				} else {
					sceneUsingNameHandle_ = targetScene->second;
					isSceneChange = true;
				}
			}
		}
		if (!isSceneChange) return;

		sceneUsing_.reset();
	}

	sceneUsing_ = sceneFactory_->CreateScene(sceneUsingNameHandle_);

	/// ③ シーンごとのメニュー設定
	if (defaultMenu_) {
		const bool isTitle = (sceneUsingNameHandle_ == "TITLE");
		defaultMenu_->SetCanOpen(!isTitle && sceneUsingNameHandle_ != "ENGINE_TITLE");   // タイトルは Esc で開かない
		defaultMenu_->SetIsTitle(isTitle);

		/// タイトルには「設定」から開けるようにメニューを渡す
		if (isTitle) {
			if (auto* title = dynamic_cast<SceneTitle*>(sceneUsing_.get())) {
				title->SetMenu(defaultMenu_.get());
			}
		}
	}
}


void SceneManager::Update() {


	SceneChanger();

	defaultMenu_->Update();

	bool frozen = false;

#ifdef _DEBUG
	/// CheatEngine部分関連
	if (cheatEngine_) {
		if (cheatEngine_->IsSceneFrozen()) {
			frozen = true;
		}
	}
#endif

	/// defaultMenu部分関連
	if (defaultMenu_) {
		if (defaultMenu_->GetIsPause()) {
			frozen = true;
		}
	}

	/// =========== 実際更新する部分 ========== ///
	if (!frozen) {
		if (sceneUsing_ != nullptr) sceneUsing_->Update();
	}
}

void SceneManager::Render() {

	if (sceneUsing_ != nullptr) {
		sceneUsing_->Draw();
	} else {
	}

#ifdef _DEBUG
	if (cheatEngine_) {
		cheatEngine_->Draw();
	}
#endif

	defaultMenu_->Draw();

#ifdef USE_IMGUI
	//ImGuiPart();
#endif
}

void SceneManager::ClearStage() {
	for (auto& ptr : stage) {
		ptr = false;
	}
	sceneUsing_.reset();
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
