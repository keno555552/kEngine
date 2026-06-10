#include "SceneTitle.h"


SceneTitle::SceneTitle(kEngine* system) {
	system_ = system;
	debugCamera_ = system_->CreateDebugCamera();
	camera_ = system_->CreateCamera();
	camera_.lock()->Move(Vector3(0.0f, 0.5f, -15.0f));
	camera_.lock()->Rotate(Vector3(0.182f, 0, 0));

	/// =========== ���\�[�X���[�h ============///

	/// ���\�[�X�n���h��

	MH_skydome_ = system_->SetModelObj("kEngine/EngineAssets/TemplateResource/object/skydome/skydome.obj");
	MH_ground_ = system_->SetModelObj("GAME/resources/Object/ground/ground.obj");

	TH_buleSkySkydome_ = system_->LoadTexture("kEngine/EngineAssets/texture/sky/bluesky.png");
	TH_title = system_->LoadTexture("kEngine/EngineAssets/texture/title.png");

	TH_startButton_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/startButton.png");
	TH_settingButton_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/settingButton.png");
	TH_quitButton_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/quitButton.png");
	TH_buttonBack_notSelect_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/buttonNotSelect.png");
	TH_buttonBack_Select_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/buttonSelect.png");

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


	title_->IntObject(system_);
	title_->CreateDefaultData();
	title_->mainPosition.transform.translate = Vector3(395.0f, 64.0f, 0.0f);
	title_->objectParts_[0].materialConfig->textureHandle = TH_title;
	title_->Update(nullptr);

	startButton_->IntObject(system_);
	startButton_->CreateDefaultData();
	startButton_->CreateDefaultData();
	startButton_->mainPosition.transform.translate = Vector3(112.0f, 402.0f, 0.0f);
	startButton_->objectParts_[1].materialConfig->textureHandle = TH_startButton_;
	//startButton_->objectParts_[1].transform.translate.z = -1;
	startButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_Select_;

	settingButton_->IntObject(system_);
	settingButton_->CreateDefaultData();
	settingButton_->CreateDefaultData();
	settingButton_->mainPosition.transform.translate = Vector3(112.0f, 495.0f, 0.0f);
	settingButton_->objectParts_[1].materialConfig->textureHandle = TH_settingButton_;
	//settingButton_->objectParts_[1].transform.translate.z = -1;
	settingButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_notSelect_;

	quitButton_->IntObject(system_);
	quitButton_->CreateDefaultData();
	quitButton_->CreateDefaultData();
	quitButton_->mainPosition.transform.translate = Vector3(112.0f, 597.0f, 0.0f);
	quitButton_->objectParts_[1].materialConfig->textureHandle = TH_quitButton_;
	//quitButton_->objectParts_[1].transform.translate.z = -1;
	quitButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_notSelect_;

	menuRun[0] = [this]() {this->SelectStart(); };
	menuRun[1] = [this]() {this->SelectSetting(); };
	menuRun[2] = [this]() {this->SelectQuit(); };

	/// BGM�Đ�
	if (!system_->SoundIsPlaying(SH_BGM_)) {
		system_->SoundPlayBGM(SH_BGM_, 0.4f);
	}
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

}

void SceneTitle::Update() {

	CameraPart();

	UpdateSelect();

}

void SceneTitle::Draw() {

	skydome_->Draw();
	ground_->Draw();

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

	ImGui::Begin("TitleScene");
	ImGui::SliderFloat3("GroundPos", &ground_->mainPosition.transform.translate.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("GroundUVScale", &ground_->objectParts_[0].materialConfig->uvScale.x, -1.0f, 1.0f);
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

void SceneTitle::UpdateSelect() {
	bool isUp = system_->GetTriggerOn(DIK_W);
	bool isDown = system_->GetTriggerOn(DIK_S);
	bool isSelect = system_->GetTriggerOn(DIK_SPACE);

	if (!isUp && !isDown && !isSelect)return;

	if (isUp && selectedButtonhandle > START) {
		selectedButtonhandle--;
		system_->SoundPlaySE(SH_Select_);
	} else if (isDown && selectedButtonhandle < QUIT) {
		selectedButtonhandle++;
		system_->SoundPlaySE(SH_Select_);
	}

	startButton_->objectParts_[0].materialConfig->textureHandle = (selectedButtonhandle == START) ? TH_buttonBack_Select_ : TH_buttonBack_notSelect_;
	settingButton_->objectParts_[0].materialConfig->textureHandle = (selectedButtonhandle == SETTING) ? TH_buttonBack_Select_ : TH_buttonBack_notSelect_;
	quitButton_->objectParts_[0].materialConfig->textureHandle = (selectedButtonhandle == QUIT) ? TH_buttonBack_Select_ : TH_buttonBack_notSelect_;
	//startButton_->objectParts_[1].materialConfig->textureHandle = (selectedButtonhandle == START) ? TH_buttonBack_notSelect_ : TH_buttonBack_Select_;
	//settingButton_->objectParts_[1].materialConfig->textureHandle = (selectedButtonhandle == SETTING) ? TH_buttonBack_notSelect_ : TH_buttonBack_Select_;
	//quitButton_->objectParts_[1].materialConfig->textureHandle = (selectedButtonhandle == QUIT) ? TH_buttonBack_notSelect_ : TH_buttonBack_Select_;
	//if(selectedButtonhandle == START )startButton_->objectParts_[0].materialConfig->textureHandle = TH_title;
	//if(selectedButtonhandle == SETTING)startButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_Select_;
	//if(selectedButtonhandle == QUIT)startButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_notSelect_;
	
	if (isSelect) {
		if (selectedButtonhandle != SETTING) {
			system_->SoundStop(SH_BGM_);
		}
		system_->SoundPlaySE(SH_Decide_);
		menuRun[selectedButtonhandle]();

	}
}

void SceneTitle::SelectStart() {
	outcome_ = SceneOutcome::NEXT;
}

void SceneTitle::SelectSetting() {
	if (defaultMenu_) {
		defaultMenu_->SetClickOpenMenu(true);
	}
}

void SceneTitle::SelectQuit() {
	outcome_ = SceneOutcome::EXIT;
}
