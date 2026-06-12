#include "sceneLose.h"

SceneLose::SceneLose(kEngine* kEngine) {
	system_ = kEngine;

	/// =========== リソースハンドル ============///
	/// リソースハンドル
	std::string basePath = "resources/";
	std::string texturePath = "texture/";
	std::string resultPath = "Result/";

	std::string soundPath = "sound/";
	std::string BGMPath = "BGM/";
	std::string SEPath = "SE/";

	TH_lose_ = kEngine->LoadTexture((basePath + texturePath + resultPath + "Result_Lose.png").c_str());
	TH_BG_ = kEngine->LoadTexture((basePath + texturePath + resultPath + "Result_BG.png").c_str());
	TH_pressKey_ = kEngine->LoadTexture((basePath + texturePath + resultPath + "Result_BackToTitle.png").c_str());

	SH_BGM_ = kEngine->SoundLoadSE((basePath + soundPath + BGMPath + "Lose.wav").c_str());

	/// =========== ゲームオブジェクト ==========///

	lose_ = new SimpleSprite;
	lose_->IntObject(kEngine);
	lose_->CreateDefaultData();
	lose_->mainPosition.transform.translate = { 395.0f, 128.0f, 0.0f };
	lose_->objectParts_[0].materialConfig->textureHandle = TH_lose_;
	lose_->Update(nullptr);

	BG_ = new SimpleSprite;
	BG_->IntObject(kEngine);
	BG_->CreateDefaultData();
	BG_->mainPosition.transform.translate = { 0.0f, 0.0f, 0.0f };
	BG_->objectParts_[0].materialConfig->textureHandle = TH_BG_;
	BG_->Update(nullptr);

	pressKey_ = new SimpleSprite;
	pressKey_->IntObject(kEngine);
	pressKey_->CreateDefaultData();
	pressKey_->mainPosition.transform.translate = { 248.0f, 444.0f, 0.0f };
	pressKey_->objectParts_[0].materialConfig->textureHandle = TH_pressKey_;
	pressKey_->Update(nullptr);

	system_->SoundPlayBGM(SH_BGM_, 0.5f);
}

SceneLose::~SceneLose() {
	system_->SoundStop(SH_BGM_);

	delete lose_, lose_ = nullptr;
	delete BG_, BG_ = nullptr;
	delete pressKey_, pressKey_ = nullptr;
}

void SceneLose::Update() {

	lose_->Update(nullptr);
	BG_->Update(nullptr);
	pressKey_->Update(nullptr);

	if (system_->GetTriggerOn(DIK_SPACE)) {
		system_->SoundStop(SH_BGM_);
		outcome_ = SceneOutcome::NEXT;
	}

}

void SceneLose::Draw() {

	BG_->Draw();
	lose_->Draw();
	pressKey_->Draw();

}