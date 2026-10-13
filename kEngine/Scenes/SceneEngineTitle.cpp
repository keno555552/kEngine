#include "SceneEngineTitle.h"


SceneEngineTitle::SceneEngineTitle(kEngine* system) {
	system_ = system;
	/// =========== リソースハンドル ============///

	/// テキスチャーハンドル
	TH_title = system_->LoadTexture("kEngine/EngineAssets/texture/EngineLogo_v1.png");

	/// サウンドハンドル
	SH_BGM_ = system_->SoundLoadSE("GAME/resources/sound/BGM/Title.wav");
	SH_Decide_ = system_->SoundLoadSE("GAME/resources/sound/SE/menuChoose.wav");
	/// ===========Q[IuWFNg ==========///
	Vector2 windowSize;
	Vector2 windowCenter;
	windowSize.x = (int)config::GetClientWidth();
	windowSize.y = (int)config::GetClientHeight();
	windowCenter.x = windowSize.x / 2;
	windowCenter.y = windowSize.y / 2;


	title_ = std::make_unique<SimpleSprite>();
	title_->IntObject(system_);
	title_->CreateDefaultData();
	title_->mainPosition.transform.translate = { windowCenter.x, windowCenter.y, 0.0f };
	title_->objectParts_[0].materialConfig->textureColor.w = 0.999f;
	title_->objectParts_[0].materialConfig->textureHandle = TH_title;
	title_->objectParts_[0].anchorPoint = {320.0f, 180.0f};
	title_->Update(nullptr);

	BG_ = std::make_unique<SimpleSprite>();
	BG_->IntObject(system_);
	BG_->CreateDefaultData();
	BG_->objectParts_[0].materialConfig->textureHandle = config::default_White5x5_TextureHandle_;
	BG_->objectParts_[0].materialConfig->textureColor = BGColor_;
	BG_->objectParts_[0].conerData = { 0.0f,0.0f,
									   0.0f,windowSize.y,
									   windowSize.x,windowSize.y,
									   windowSize.x ,0.0f };
	BG_->Update(nullptr);

	/// BGM
	//if (!system_->SoundIsPlaying(SH_BGM_)) {
	//	system_->SoundPlayBGM(SH_BGM_, 0.4f);
	//}
}

SceneEngineTitle::~SceneEngineTitle() {

	system_->SoundStop(SH_BGM_);

	title_.release();
}

void SceneEngineTitle::Update() {

}

void SceneEngineTitle::Draw() {

	BG_->Draw();
	title_->Draw();

#ifdef USE_IMGUI
	/// ImGui
	ImGuiPart();
#endif
}

#ifdef USE_IMGUI
void SceneEngineTitle::ImGuiPart() {

	ImGui::Begin("Title");
	ImGui::SliderFloat2("Pos", &title_->mainPosition.transform.translate.x, -10.0f, 10.0f);

	ImGui::End();
}
#endif
