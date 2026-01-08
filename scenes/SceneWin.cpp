#include "SceneWin.h"

SceneWin::SceneWin(kEngine* kEngine) {
	system_ = kEngine;

	/// =========== リソースハンドル ============///
	/// リソースハンドル
	std::string basePath = "resources/";
	std::string texturePath = "texture/";
	std::string resultPath = "Result/";

	std::string soundPath = "sound/";
	std::string BGMPath = "BGM/";
	std::string SEPath = "SE/";

	TH_win_ = kEngine->LoadTextrue((basePath + texturePath + resultPath + "Result_Win.png").c_str());
	TH_BG_ = kEngine->LoadTextrue((basePath + texturePath + resultPath + "Result_BG.png").c_str());
	TH_pressKey_ = kEngine->LoadTextrue((basePath + texturePath + resultPath + "Result_BackToTitle.png").c_str());

	SH_BGM_ = kEngine->SoundLoadSE((basePath + soundPath + BGMPath + "Win.wav").c_str());

	/// =========== ゲームオブジェクト ==========///

	win_ = new SimpleSprite;
	win_->IntObject(kEngine);
	win_->CreateDefaultData();
	win_->mainPosition.transform.translate = Vector3(421.0f, 218.0f, 0.0f);
	win_->objectParts_[0].materialConfig->textureHandle = TH_win_;
	win_->Update(nullptr);

	BG_ = new SimpleSprite;
	BG_->IntObject(kEngine);
	BG_->CreateDefaultData();
	BG_->mainPosition.transform.translate = Vector3(0.0f, 0.0f, 0.0f);
	BG_->objectParts_[0].materialConfig->textureHandle = TH_BG_;
	BG_->Update(nullptr);

	pressKey_ = new SimpleSprite;
	pressKey_->IntObject(kEngine);
	pressKey_->CreateDefaultData();
	pressKey_->mainPosition.transform.translate = Vector3(248.0f, 444.0f, 0.0f);
	pressKey_->objectParts_[0].materialConfig->textureHandle = TH_pressKey_;
	pressKey_->Update(nullptr);

	system_->SoundPlayBGM(SH_BGM_, 0.5f);
}

SceneWin::~SceneWin() {
	system_->SoundStop(SH_BGM_);

	delete win_, win_ = nullptr;
	delete BG_, BG_ = nullptr;
	delete pressKey_, pressKey_ = nullptr;
}

void SceneWin::Update() {

	win_->Update(nullptr);
	BG_->Update(nullptr);
	pressKey_->Update(nullptr);

	if (system_->GetTriggerOn(DIK_SPACE)) {
		system_->SoundStop(SH_BGM_);
		ChangeNextStage(SceneNum::S_TITLE);
	}

}

void SceneWin::Draw() {

	BG_->Draw();
	win_->Draw();
	pressKey_->Draw();

}
