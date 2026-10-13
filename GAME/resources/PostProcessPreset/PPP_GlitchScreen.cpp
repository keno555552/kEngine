#include "PPP_GlitchScreen.h"
#include "kEngine.h"

PPP_GlitchScreen::PPP_GlitchScreen(kEngine* engine) : engine_(engine) {

	/// 
	MakeNoise(noiseCommand_, RandomNoiseType::WhiteNoise, 0.25f, 1.0f);
	colorGradingCommand_.guardColor = { 0.0f,0.082f,0.771f };
	colorGradingCommand_.guardAmount = 0.153f;
	MakeBoxBlur(blurCommand_, 3);
	vignetteCommand_.vignetteCenter = { 0.5f, 0.5f };
	vignetteCommand_.vignetteRadius = 1.0f;
	vignetteCommand_.vignetteSoftness = 0.05f;
	vignetteCommand_.vignetteIntensity = 5.7f;


	noisePassHandle_ = engine_->GetPostProcessRunner()->SetPassNoise(0, noiseCommand_);
	ColorGradingPassHandle_ = engine_->GetPostProcessRunner()->SetPassColorGrading(0, colorGradingCommand_);
	BlurPassHandle_ = engine_->GetPostProcessRunner()->SetPassBlur(0, blurCommand_);
	VignettePassHandle_ = engine_->GetPostProcessRunner()->SetPassVignette(0, vignetteCommand_);

}

void PPP_GlitchScreen::Update() {

	noiseCommand_.randomNoiseTime += 0.01f;
	engine_->GetPostProcessRunner()->SetPassCommand(0, noisePassHandle_, noiseCommand_);
	engine_->GetPostProcessRunner()->SetPassCommand(0, ColorGradingPassHandle_, colorGradingCommand_);
	engine_->GetPostProcessRunner()->SetPassCommand(0, BlurPassHandle_, blurCommand_);
	engine_->GetPostProcessRunner()->SetPassCommand(0, VignettePassHandle_, vignetteCommand_);

}

void PPP_GlitchScreen::OffAllPass() {
	engine_->GetPostProcessRunner()->ClearLayer(0);
}

void PPP_GlitchScreen::OnAllPass() {

	noisePassHandle_ = engine_->GetPostProcessRunner()->SetPassNoise(0, noiseCommand_);
	ColorGradingPassHandle_ = engine_->GetPostProcessRunner()->SetPassColorGrading(0, colorGradingCommand_);
	BlurPassHandle_ = engine_->GetPostProcessRunner()->SetPassBlur(0, blurCommand_);
	VignettePassHandle_ = engine_->GetPostProcessRunner()->SetPassVignette(0, vignetteCommand_);
}

void PPP_GlitchScreen::ResetAllPass() {

	MakeNoise(noiseCommand_, RandomNoiseType::WhiteNoise, 0.25f, 1.0f);
	colorGradingCommand_.guardColor = { 0.0f,0.1f,0.2f };
	colorGradingCommand_.guardAmount = 0.6f;
	MakeBoxBlur(blurCommand_, 3);
	vignetteCommand_.vignetteCenter = { 0.5f, 0.5f };
	vignetteCommand_.vignetteRadius = 0.75f;
	vignetteCommand_.vignetteSoftness = 0.045f;
	vignetteCommand_.vignetteIntensity = 0.35f;

	Update();

}
