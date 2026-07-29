#include "PPP_GlitchScreen.h"
#include "kEngine.h"

PPP_GlitchScreen::PPP_GlitchScreen(kEngine* engine) : engine_(engine) {

	/// 
	MakeNoise(noiseCommand_, RandomNoiseType::WhiteNoise, 0.25f, 1.0f);
	ColorGradingCommand_.guardColor = { 0.0f,0.082f,0.771f };
	ColorGradingCommand_.guardAmount = 0.153f;
	MakeBoxBlur(BlurCommand_, 3);
	VignetteCommand_.vignetteCenter = { 0.5f, 0.5f };
	VignetteCommand_.vignetteRadius = 1.0f;
	VignetteCommand_.vignetteSoftness = 0.05f;
	VignetteCommand_.vignetteIntensity = 5.7f;


	noisePassHandle_ = engine_->GetPostProcessRunner()->SetPassNoise(0, noiseCommand_);
	ColorGradingPassHandle_ = engine_->GetPostProcessRunner()->SetPassColorGrading(0, ColorGradingCommand_);
	BlurPassHandle_ = engine_->GetPostProcessRunner()->SetPassBlur(0, BlurCommand_);
	VignettePassHandle_ = engine_->GetPostProcessRunner()->SetPassVignette(0, VignetteCommand_);

}

void PPP_GlitchScreen::Update() {

	noiseCommand_.randomNoiseTime += 0.01f;
	engine_->GetPostProcessRunner()->SetPassCommand(0, noisePassHandle_, noiseCommand_);
	engine_->GetPostProcessRunner()->SetPassCommand(0, ColorGradingPassHandle_, ColorGradingCommand_);
	engine_->GetPostProcessRunner()->SetPassCommand(0, BlurPassHandle_, BlurCommand_);
	engine_->GetPostProcessRunner()->SetPassCommand(0, VignettePassHandle_, VignetteCommand_);

}

void PPP_GlitchScreen::OffAllPass() {
	engine_->GetPostProcessRunner()->ClearLayer(0);
}

void PPP_GlitchScreen::OnAllPass() {

	noisePassHandle_ = engine_->GetPostProcessRunner()->SetPassNoise(0, noiseCommand_);
	ColorGradingPassHandle_ = engine_->GetPostProcessRunner()->SetPassColorGrading(0, ColorGradingCommand_);
	BlurPassHandle_ = engine_->GetPostProcessRunner()->SetPassBlur(0, BlurCommand_);
	VignettePassHandle_ = engine_->GetPostProcessRunner()->SetPassVignette(0, VignetteCommand_);
}

void PPP_GlitchScreen::ResetAllPass() {

	MakeNoise(noiseCommand_, RandomNoiseType::WhiteNoise, 0.25f, 1.0f);
	ColorGradingCommand_.guardColor = { 0.0f,0.1f,0.2f };
	ColorGradingCommand_.guardAmount = 0.6f;
	MakeBoxBlur(BlurCommand_, 3);
	VignetteCommand_.vignetteCenter = { 0.5f, 0.5f };
	VignetteCommand_.vignetteRadius = 0.75f;
	VignetteCommand_.vignetteSoftness = 0.045f;
	VignetteCommand_.vignetteIntensity = 0.35f;

	Update();

}
