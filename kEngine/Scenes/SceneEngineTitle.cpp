#include "SceneEngineTitle.h"


SceneEngineTitle::SceneEngineTitle(kEngine* system) {
	system_ = system;

	phaseChangeTimer_ = new Timer;
	colorGradingCommand_.guardColor = { 0.0f,0.0f,0.0f };
	colorGradingCommand_.guardAmount = 0.0f;
	ColorGradingPassHandle_ = system_->GetPostProcessRunner()->SetPassColorGrading(0, colorGradingCommand_);

	/// =========== リソースハンドル ============///

	/// テキスチャーハンドル
	TH_title = system_->LoadTexture("kEngine/EngineAssets/texture/EngineLogo_v1.png");

	/// サウンドハンドル
	SH_BGM_ = system_->SoundLoadSE("GAME/resources/sound/BGM/Title.wav");
	SH_Decide_ = system_->SoundLoadSE("GAME/resources/sound/SE/menuChoose.wav");
	/// ===========Q[IuWFNg ==========///
	Vector2 windowSize;
	Vector2 windowCenter;
	windowSize.x = (float)config::GetClientWidth();
	windowSize.y = (float)config::GetClientHeight();
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

	system_->GetPostProcessRunner()->ClearLayer(0);
}

void SceneEngineTitle::Update() {
	PhaseUpdate();
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

void SceneEngineTitle::PhaseUpdate() {

	float rate{};

	switch (phase_) {
	case ScenePhase::PREPARE:

		phase_ = ScenePhase::ENTRY;

	case ScenePhase::ENTRY:

		/// タイマー初期化
		if (phaseChangeTimer_->maxTime_ == 0.0f) {
			phaseChangeTimer_->Init0(entryTime_, system_->GetTimeManager());
		}
		/// 線方補間
		rate = phaseChangeTimer_->linearity();
		/// circleに代入
		colorGradingCommand_.guardAmount = 1.0f - rate;
		system_->GetPostProcessRunner()->SetPassCommand(0, ColorGradingPassHandle_, colorGradingCommand_);


		/// カンター行動
		if (phaseChangeTimer_->ToMix()) {
			phase_ = ScenePhase::PROGRESS;
		}

		break;

	case ScenePhase::PROGRESS:

		/// タイマー初期化
		if (phaseChangeTimer_->GetIsMax()) {
			phaseChangeTimer_->Init0(showTime_, system_->GetTimeManager());
		}

		/// カンター行動
		if (phaseChangeTimer_->ToMix()) {
			phase_ = ScenePhase::TRANSITION;
		}

		break;
	case ScenePhase::TRANSITION:
		/// タイマー初期化
		if (phaseChangeTimer_->GetIsMax()) {
			phaseChangeTimer_->Init0(entryTime_, system_->GetTimeManager());
		}
		/// 線方補間
		rate = phaseChangeTimer_->linearity();
		/// circleに代入
		colorGradingCommand_.guardAmount = rate;
		system_->GetPostProcessRunner()->SetPassCommand(0, ColorGradingPassHandle_, colorGradingCommand_);

		/// カンター行動
		if (phaseChangeTimer_->ToMix()) {
			outcome_ = SceneOutcome::NEXT;
		}
		break;
	}

	bool isMousePush = system_->GetMouseIsPush(0) || system_->GetMouseIsPush(1) || system_->GetMouseIsPush(2);
	bool isKeysPush =
		system_->GetIsPush(DIK_RETURN) || 
		system_->GetIsPush(DIK_ESCAPE) ||
		system_->GetIsPush(DIK_SPACE) || 
		system_->GetIsPush(DIK_A) ||
		system_->GetIsPush(DIK_S) ||
		system_->GetIsPush(DIK_D) || 
		system_->GetIsPush(DIK_F) ||
		system_->GetIsPush(DIK_R);

	if (isKeysPush || isMousePush) {
		outcome_ = SceneOutcome::NEXT;
	}
}

#endif
