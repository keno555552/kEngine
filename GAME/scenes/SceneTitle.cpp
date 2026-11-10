#include "SceneTitle.h"


SceneTitle::SceneTitle(kEngine* system) {
	system_ = system;
	debugCamera_ = system_->CreateDebugCamera();
	debugCamera_.lock()->SetTranslate({ 0.0f, 0.0f, -8.333f });

	camera_ = system_->CreateCamera();
	camera_.lock()->SetTranslate({ 0.0f, 0.0f, -8.333f });

	phaseChangeTimer_ = new Timer;

	/// =========== 後処理初期化 ==============///

	colorGradingCommand_.guardColor = { 0.0f,0.0f,0.0f };
	colorGradingCommand_.guardAmount = 0.0f;
	ColorGradingPassHandle_ = system_->GetPostProcessRunner()->SetPassColorGrading(0, colorGradingCommand_);

	/// =========== リソースハンドル ============///

	/// モデルハンドル
	MH_skydome_ = system_->SetModelObj("kEngine/EngineAssets/TemplateResource/object/skydome/skydome.obj");
	MH_ground_ = system_->SetModelObj("GAME/resources/Object/ground/ground.obj");
	MH_driller_ = system_->SetModelObj("GAME/resources/Object/driller/driller.gltf");

	/// テキスチャーハンドル
	TH_buleSkySkydome_ = system_->LoadTexture("kEngine/EngineAssets/texture/sky/bluesky.png");
	TH_title = system_->LoadTexture("kEngine/EngineAssets/texture/title.png");
	TH_background_ = system_->LoadTexture("GAME/resources/texture/stone.png");

	TH_startButton_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/startButton.png");
	TH_settingButton_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/settingButton.png");
	TH_quitButton_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/quitButton.png");
	TH_buttonBack_notSelect_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/buttonNotSelect.png");
	TH_buttonBack_Select_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/buttonSelect.png");

	/// サウンドハンドル
	SH_BGM_ = system_->SoundLoadSE("GAME/resources/sound/BGM/Title.wav");
	SH_Select_ = system_->SoundLoadSE("GAME/resources/sound/SE/menuSelect.wav");
	SH_Decide_ = system_->SoundLoadSE("GAME/resources/sound/SE/menuChoose.wav");
	/// ===========Q[IuWFNg ==========///

	skydome_ = std::make_unique<Object>();
	skydome_->IntObject(system_);
	skydome_->CreateModelData(MH_skydome_);
	skydome_->objectParts_[0].materialConfig->enableLighting = false;
	skydome_->objectParts_[0].materialConfig->useModelTexture = false;
	skydome_->objectParts_[0].materialConfig->textureHandle = TH_buleSkySkydome_;

	ground_ = std::make_unique<Object>();
	ground_->IntObject(system_);
	ground_->CreateModelData(MH_ground_);
	ground_->objectParts_[0].materialConfig->uvScale = { 100.0f,100.0f,0 };

	background_ = std::make_unique<Object>();
	background_->IntObject(system_);
	background_->CreateDefaultData();
	background_->modelHandle_ = config::default_Plane_MeshBufferHandle_;
	background_->objectParts_[0].materialConfig->enableLighting = true;
	background_->objectParts_[0].materialConfig->useModelTexture = false;
	background_->objectParts_[0].materialConfig->textureHandle = TH_background_;
	background_->mainPosition.transform.scale = { 18.0f,18.0f,1.0f };
	background_->mainPosition.transform.rotate = { -0.698f, -0.465f, 0.0f };
	background_->objectParts_[0].materialConfig->uvScale = { 20.0f, 20.0f, 1.0f };
	movingSpeed_ = 0.03f;

	title_->IntObject(system_);
	title_->CreateDefaultData();
	title_->mainPosition.transform.translate = { 700.0f, 80.0f, 0.0f };
	title_->objectParts_[0].materialConfig->textureHandle = TH_title;
	title_->Update(nullptr);


	drillerStartPositon_.translate = { -3.222f, 2.462f, -1.0f };
	drillerStartPositon_.rotate = { -0.483f, 0.314f, -0.628f };
	drillerStartPositon_.scale = { 0.7f, 0.7f, 0.7f };

	drillerHoldPosition_.translate = { -1.0f, -0.5f, -1.0f };
	drillerHoldPosition_.rotate = { -0.483f, 0.314f, -0.628f };
	drillerHoldPosition_.scale = { 0.7f, 0.7f, 0.7f };

	drillerEndPosition_.translate = { 2.333f, -4.943f, -1.0f };
	drillerEndPosition_.rotate = { -0.483f, 0.314f, -0.628f };
	drillerEndPosition_.scale = { 0.7f, 0.7f, 0.7f };



	driller_ = std::make_unique<Driller>(system_, Vector3{ 0.0f,0.0f,0.0f });
	driller_->CreateModelData(MH_driller_);
	driller_->mainPosition.transform = drillerStartPositon_;
	driller_->IsTitle(true);


	buttonStartX_ = 995.0f;
	startButton_->IntObject(system_);
	startButton_->CreateDefaultData();
	startButton_->CreateDefaultData();
	startButton_->mainPosition.transform.translate = { buttonStartX_, 402.0f, 0.0f };
	startButton_->objectParts_[1].materialConfig->textureHandle = TH_startButton_;
	startButton_->objectParts_[1].materialConfig->textureColor.w = 0.99f;
	startButton_->objectParts_[1].transform.translate = { 0.0f, 0.0f, 2.0f };
	startButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_Select_;
	startButton_->objectParts_[0].transform.translate = { 0.0f, 0.0f, 1.0f };
	startButton_->objectParts_[0].materialConfig->textureColor.w = 0.99f;

	settingButton_->IntObject(system_);
	settingButton_->CreateDefaultData();
	settingButton_->CreateDefaultData();
	settingButton_->mainPosition.transform.translate = { buttonStartX_, 495.0f, 0.0f };
	settingButton_->objectParts_[1].materialConfig->textureHandle = TH_settingButton_;
	settingButton_->objectParts_[1].materialConfig->textureColor.w = 0.99f;
	settingButton_->objectParts_[1].transform.translate = { 0.0f, 0.0f, 4.0f };
	settingButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_notSelect_;
	settingButton_->objectParts_[0].materialConfig->textureColor.w = 0.99f;
	settingButton_->objectParts_[0].transform.translate = { 0.0f, 0.0f, 3.0f };

	quitButton_->IntObject(system_);
	quitButton_->CreateDefaultData();
	quitButton_->CreateDefaultData();
	quitButton_->mainPosition.transform.translate = { buttonStartX_, 597.0f, 0.0f };
	quitButton_->objectParts_[1].materialConfig->textureHandle = TH_quitButton_;
	quitButton_->objectParts_[1].materialConfig->textureColor.w = 0.99f;
	quitButton_->objectParts_[1].transform.translate = { 0.0f, 0.0f, 6.0f };
	quitButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_notSelect_;
	quitButton_->objectParts_[0].materialConfig->textureColor.w = 0.99f;
	quitButton_->objectParts_[0].transform.translate = { 0.0f, 0.0f, 5.0f };

	menuRun[0] = [this]() {this->SelectStart(); };
	menuRun[1] = [this]() {this->SelectSetting(); };
	menuRun[2] = [this]() {this->SelectQuit(); };

	/// BGM
	//if (!system_->SoundIsPlaying(SH_BGM_)) {
	//	system_->SoundPlayBGM(SH_BGM_, 0.4f);
	//}
}

SceneTitle::~SceneTitle() {

	system_->SoundStop(SH_BGM_);

	system_->DestroyCamera(camera_);
	system_->DestroyCamera(debugCamera_);
	delete title_;

	SimpleSprite* title_ = new SimpleSprite;

	delete startButton_;
	delete settingButton_;
	delete quitButton_;

	system_->GetPostProcessRunner()->ClearLayer(0);
}

void SceneTitle::Update() {

	PhaseUpdate();

	CameraPart();

	UpdateSelect();

	driller_->Update(nullptr);
	BackgroundMove();

	startButton_->mainPosition.transform.translate.x = buttonStartX_;
	settingButton_->mainPosition.transform.translate.x = buttonStartX_;
	quitButton_->mainPosition.transform.translate.x = buttonStartX_;
}

void SceneTitle::Draw() {

	skydome_->Draw();
	//ground_->Draw();
	driller_->Draw();
	background_->Draw();

	title_->Draw();
	startButton_->Draw();
	settingButton_->Draw();
	quitButton_->Draw();


#ifdef USE_IMGUI
	/// ImGui����
	ImGuiPart();
#endif
}

#ifdef USE_IMGUI
void SceneTitle::ImGuiPart() {

	Transform debugCamTran = debugCamera_.lock()->GetTransform();
	Transform camTran = camera_.lock()->GetTransform();

	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse", &useDebugCamera);
	ImGui::SliderFloat3("DebugCameraPos", &debugCamTran.translate.x, -50.0f, 50.0f);
	ImGui::SliderFloat3("CameraPos", &camTran.translate.x, -50.0f, 50.0f);
	ImGui::SliderFloat3("CameraRotato", &camTran.rotate.x, -2.0f, 2.0f);
	ImGui::End();

	ImGui::Begin("Objects");
	ImGui::SliderFloat3("GroundPos", &ground_->mainPosition.transform.translate.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("GroundUVScale", &ground_->objectParts_[0].materialConfig->uvScale.x, -1.0f, 1.0f);
	ImGui::Separator();
	ImGui::SliderFloat3("BackgroundPos", &background_->mainPosition.transform.translate.x, -10.0f, 10.0f);
	float backgroundScale = background_->mainPosition.transform.scale.x;
	ImGui::SliderFloat("BackgroundScale", &backgroundScale, -20.0f, 20.0f);
	background_->mainPosition.transform.scale.x = backgroundScale;
	background_->mainPosition.transform.scale.y = backgroundScale;
	ImGui::SliderFloat3("BackgroundRotate", &background_->mainPosition.transform.rotate.x, -3.14f, 3.14f);
	ImGui::SliderFloat3("BackgroundUVScale", &background_->objectParts_[0].materialConfig->uvScale.x, -20.0f, 20.0f);
	ImGui::SliderFloat3("BackgroundUVTranslate", &background_->objectParts_[0].materialConfig->uvTranslate.x, -20.0f, 20.0f);
	ImGui::SliderFloat("BackgroundMovingSpeed", &movingSpeed_, -1.0f, 1.0f);
	ImGui::Separator();
	ImGui::SliderFloat3("DrillerPos", &driller_->mainPosition.transform.translate.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("DrillerRotate", &driller_->mainPosition.transform.rotate.x, -3.14f, 3.14f);
	ImGui::SliderFloat3("DrillerScale", &driller_->mainPosition.transform.scale.x, -10.0f, 10.0f);
	ImGui::Separator();
	ImGui::SliderFloat3("TitlePos", &title_->mainPosition.transform.translate.x, -1280.0f, 1280.0f);
	ImGui::SliderFloat("ButtonStartPosX", &buttonStartX_, -1280.0f, 1280.0f);

	ImGui::End();

	debugCamera_.lock()->SetCamera(debugCamTran);
	camera_.lock()->SetCamera(camTran);
}
#endif

void SceneTitle::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
		debugCamera_.lock()->MouseControlUpdate();
	} else {
		usingCamera_ = camera_;
	}
	system_->SetCamera(usingCamera_);
}

void SceneTitle::PhaseUpdate() {

	Transform drillerNowPositon_{};
	float rate{};
	Transform startPos {};
	Transform endPos {};

	switch (phase_) {
	case ScenePhase::PREPARE:

		phase_ = ScenePhase::ENTRY;

	case ScenePhase::ENTRY:

		if (phaseChangeTimer_->maxTime_ == 0.0f) {
			phaseChangeTimer_->Init0(entryTime_, system_->GetTimeManager());
		}

		rate = phaseChangeTimer_->easyIn(entryRate_);
		startPos = drillerStartPositon_;
		endPos = drillerHoldPosition_;

		drillerNowPositon_ = LinearTransform(startPos, endPos, rate);

		driller_->mainPosition.transform = drillerNowPositon_;

		colorGradingCommand_.guardAmount = 1.0f - rate;
		system_->GetPostProcessRunner()->SetPassCommand(0, ColorGradingPassHandle_, colorGradingCommand_);

		if (phaseChangeTimer_->ToMix()) {
			phase_ = ScenePhase::PROGRESS;
		}

		break;

	case ScenePhase::PROGRESS:
		break;
	case ScenePhase::TRANSITION:
		if (phaseChangeTimer_->GetIsMax()) {
			phaseChangeTimer_->Init0(translationTime_, system_->GetTimeManager());
		}

		rate = phaseChangeTimer_->easyOut(entryRate_);
		startPos = drillerHoldPosition_;
		endPos = drillerEndPosition_;

		drillerNowPositon_ = LinearTransform(startPos, endPos, rate);

		driller_->mainPosition.transform = drillerNowPositon_;

		colorGradingCommand_.guardAmount = rate;
		system_->GetPostProcessRunner()->SetPassCommand(0, ColorGradingPassHandle_, colorGradingCommand_);

		if (phaseChangeTimer_->ToMix()) {
			outcome_ = SceneOutcome::NEXT;
		}
		break;
	}

	if (isEndGame_) {

		if (phaseChangeTimer_->GetIsMax()) {
			phaseChangeTimer_->Init0(translationTime_, system_->GetTimeManager());
		}

		rate = phaseChangeTimer_->linearity();
		startPos = drillerHoldPosition_;
		endPos = drillerStartPositon_;

		drillerNowPositon_ = LinearTransform(startPos, endPos, rate);

		driller_->mainPosition.transform = drillerNowPositon_;

		colorGradingCommand_.guardAmount = rate;
		system_->GetPostProcessRunner()->SetPassCommand(0, ColorGradingPassHandle_, colorGradingCommand_);

		if (phaseChangeTimer_->ToMix()) {
			outcome_ = SceneOutcome::EXIT;
		}
	}
}

void SceneTitle::BackgroundMove() {

	background_->objectParts_[0].materialConfig->uvTranslate.x += movingSpeed_;
	background_->objectParts_[0].materialConfig->uvTranslate.y += movingSpeed_;

}

void SceneTitle::UpdateSelect() {
	bool isUp = system_->GetTriggerOn(DIK_W);
	bool isDown = system_->GetTriggerOn(DIK_S);
	bool isSelect = system_->GetTriggerOn(DIK_SPACE);

	if (!isUp && !isDown && !isSelect)return;

	if (isUp && selectedButtonHandle > START) {
		selectedButtonHandle--;
		system_->SoundPlaySE(SH_Select_);
	} else if (isDown && selectedButtonHandle < QUIT) {
		selectedButtonHandle++;
		system_->SoundPlaySE(SH_Select_);
	}

	startButton_->objectParts_[0].materialConfig->textureHandle = (selectedButtonHandle == START) ? TH_buttonBack_Select_ : TH_buttonBack_notSelect_;
	settingButton_->objectParts_[0].materialConfig->textureHandle = (selectedButtonHandle == SETTING) ? TH_buttonBack_Select_ : TH_buttonBack_notSelect_;
	quitButton_->objectParts_[0].materialConfig->textureHandle = (selectedButtonHandle == QUIT) ? TH_buttonBack_Select_ : TH_buttonBack_notSelect_;

	if (isSelect) {
		if (selectedButtonHandle != SETTING) {
			system_->SoundStop(SH_BGM_);
		}
		if (selectedButtonHandle == MAX) {
			assert(0 && "selectedButtonHandle is out of range");
		}
		system_->SoundPlaySE(SH_Decide_);
		menuRun[selectedButtonHandle]();
	}
}

void SceneTitle::SelectStart() {
	phase_ = ScenePhase::TRANSITION;
}

void SceneTitle::SelectSetting() {
	if (defaultMenu_) {
		defaultMenu_->SetClickOpenMenu(true);
	}
}

void SceneTitle::SelectQuit() {
	isEndGame_ = true;
}
