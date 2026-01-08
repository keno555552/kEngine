#include "SceneTitle.h"


SceneTitle::SceneTitle(kEngine* system) {
	system_ = system;
	debugCamera_ = new DebugCamera(system);
	camera_ = new Camera;
	camera_->Move(Vector3(0.0f, 0.5f, -15.0f));
	camera_->Rotate(Vector3(0.182f, 0, 0));

	/// =========== リソースロード ============///

	/// リソースハンドル
	std::string basePath = "resources/";
	std::string templatePath = "TemplateResource/";
	std::string objectPath = "object/";
	std::string texturePath = "texture/";
	std::string blockPath = "block/";
	std::string buttonPath = "button/";
	std::string soundPath = "sound/";
	std::string BGMPath = "BGM/";
	std::string SEPath = "SE/";

	MH_skydome_ = system_->SetModelObj((basePath + templatePath + objectPath + "skydome/skydome.obj").c_str());
	MH_ground_ = system_->SetModelObj((basePath + objectPath + "ground/ground.obj").c_str());

	TH_buleSkySkydome_ = system_->LoadTextrue((basePath + texturePath + "sky/bluesky.png").c_str());
	TH_title = system_->LoadTextrue((basePath + texturePath + "title.png").c_str());

	TH_startButton_ = system_->LoadTextrue((basePath + texturePath + buttonPath + "startButton.png").c_str());
	TH_settingButton_ = system_->LoadTextrue((basePath + texturePath + buttonPath + "settingButton.png").c_str());
	TH_quitButton_ = system_->LoadTextrue((basePath + texturePath + buttonPath + "quitButton.png").c_str());
	TH_buttonBack_notSelect_ = system_->LoadTextrue((basePath + texturePath + buttonPath + "buttonNotSelect.png").c_str());
	TH_buttonBack_Select_ = system_->LoadTextrue((basePath + texturePath + buttonPath + "buttonSelect.png").c_str());

	SH_BGM_ = system_->SoundLoadSE((basePath + soundPath + BGMPath + "Title.wav").c_str());
	SH_Select_ = system_->SoundLoadSE((basePath + soundPath + SEPath + "menuSelect.wav").c_str());
	SH_Decide_ = system_->SoundLoadSE((basePath + soundPath + SEPath + "menuChoose.wav").c_str());
	/// =========== ゲームオブジェクト初期化 ==========///

	skydome_ = new Object;
	skydome_->IntObject(system_);
	skydome_->CreateModelData(MH_skydome_);
	skydome_->objectParts_[0].materialConfig->enableLighting = false;
	skydome_->objectParts_[0].materialConfig->useModelTexture = false;
	skydome_->objectParts_[0].materialConfig->textureHandle = TH_buleSkySkydome_;

	ground_ = new Object;
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
	startButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_Select_;

	settingButton_->IntObject(system_);
	settingButton_->CreateDefaultData();
	settingButton_->CreateDefaultData();
	settingButton_->mainPosition.transform.translate = Vector3(112.0f, 495.0f, 0.0f);
	settingButton_->objectParts_[1].materialConfig->textureHandle = TH_settingButton_;
	settingButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_notSelect_;

	quitButton_->IntObject(system_);
	quitButton_->CreateDefaultData();
	quitButton_->CreateDefaultData();
	quitButton_->mainPosition.transform.translate = Vector3(112.0f, 597.0f, 0.0f);
	quitButton_->objectParts_[1].materialConfig->textureHandle = TH_quitButton_;
	quitButton_->objectParts_[0].materialConfig->textureHandle = TH_buttonBack_notSelect_;

	menuRun[0] = [=]() {SceneTitle::SelectStart(); };
	menuRun[1] = [=]() {SceneTitle::SelectSetting(); };
	menuRun[2] = [=]() {SceneTitle::SelectQuit(); };

	/// BGM再生
	if(!system_->SoundIsPlaying(SH_BGM_)){
		system_->SoundPlayBGM(SH_BGM_, 0.4f);
	}
}

SceneTitle::~SceneTitle() {

	system_->SoundStop(SH_BGM_);

	delete camera_;
	delete debugCamera_;
	delete title_;
}

void SceneTitle::Update() {
	/// カメラ処理
	CameraPart();


	//if (system_->GetTriggerOn(DIK_SPACE)) {
	//	ChangeNextStage(SceneNum::S_STAGE_01);
	//}
	UpdateSelect();

	/// Skydome更新
	skydome_->Update(usingCamera_);

	/// Ground更新
	ground_->Update(usingCamera_);

	/// Sprite更新
	title_->Update(nullptr);
	startButton_->Update(nullptr);
	settingButton_->Update(nullptr);
	quitButton_->Update(nullptr);

}

void SceneTitle::Draw() {

	/// 実体処理
	skydome_->Draw();
	ground_->Draw();

	title_->Draw();
	startButton_->Draw();
	settingButton_->Draw();
	quitButton_->Draw();


#ifdef USE_IMGUI
	/// imgui処理
	ImguiPart();
#endif
}

#ifdef USE_IMGUI
void SceneTitle::ImguiPart() {

	Vector3 debugCamPos = debugCamera_->GetTransform().translate;
	Vector3 camPos = camera_->GetTransform().translate;
	Vector3 camRotate = camera_->GetTransform().rotate;

	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse", &useDebugCamera);
	ImGui::SliderFloat3("DebugCameraPos", &debugCamPos.x, -50.0f, 50.0f);
	ImGui::SliderFloat3("CameraPos", &camPos.x, -50.0f, 50.0f);
	ImGui::SliderFloat3("CameraRotato", &camRotate.x, -2.0f, 2.0f);
	ImGui::End();

	ImGui::Begin("TitleScene");
	ImGui::SliderFloat3("GroundPos", &ground_->mainPosition.transform.translate.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("GroundUVScale", &ground_->objectParts_[0].materialConfig->uvScale.x, -1.0f, 1.0f);
	ImGui::End();

	debugCamera_->SetTranslate(debugCamPos);
	camera_->SetTranslate(camPos);
	camera_->SetRotate(camRotate);

}
#endif

void SceneTitle::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
	} else {
		usingCamera_ = camera_;
	}
	usingCamera_->Update();
}

void SceneTitle::UpdateSelect() {
	bool isUp = system_->GetTriggerOn(DIK_W);
	bool isDown = system_->GetTriggerOn(DIK_S);
	bool isSelect = system_->GetTriggerOn(DIK_SPACE);

	if (!isUp && !isDown && !isSelect)return;

	/// ボタン選択処理
	if (isUp && selectedButtonhandle > START) {
		selectedButtonhandle--;
		system_->SoundPlaySE(SH_Select_);
	} else if (isDown && selectedButtonhandle < QUIT) {
		selectedButtonhandle++;
		system_->SoundPlaySE(SH_Select_);
	}

	/// ボタンハンドルに応じてテクスチャ変更
	startButton_->objectParts_[0].materialConfig->textureHandle = (selectedButtonhandle == START) ? TH_buttonBack_Select_ : TH_buttonBack_notSelect_;
	settingButton_->objectParts_[0].materialConfig->textureHandle = (selectedButtonhandle == SETTING) ? TH_buttonBack_Select_ : TH_buttonBack_notSelect_;
	quitButton_->objectParts_[0].materialConfig->textureHandle = (selectedButtonhandle == QUIT) ? TH_buttonBack_Select_ : TH_buttonBack_notSelect_;

	/// スタートボタン
	if (isSelect) {
		if (selectedButtonhandle != SETTING) {
			/// BGM停止
			system_->SoundStop(SH_BGM_);
		}
		system_->SoundPlaySE(SH_Decide_);
		menuRun[selectedButtonhandle]();

	}
}

void SceneTitle::SelectStart() {
	ChangeNextStage(SceneNum::S_STAGE_01);
}

void SceneTitle::SelectSetting() {
	if (defaultMenu_) {
		defaultMenu_->SetClickOpenMenu(true);
	}
}

void SceneTitle::SelectQuit() {
	ChangeNextStage(SceneNum::S_END);
}
