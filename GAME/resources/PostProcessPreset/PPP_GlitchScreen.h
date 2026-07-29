#pragma once
#include "Data/Render/CPUData/RenderCommand.h"

class kEngine;
class PPP_GlitchScreen {

public:
	PPP_GlitchScreen(kEngine* engine);

	RenderCommand* GetNoiseCommand() { return &noiseCommand_; }
	RenderCommand* GetColorGradingCommand() { return &ColorGradingCommand_; }
	RenderCommand* GetBlurCommand() { return &BlurCommand_; }
	RenderCommand* GetVignetteCommand() { return &VignetteCommand_; }

	void UpdateNoiseCommand(RenderCommand command) { noiseCommand_ = command; }
	void Update();

	void OffAllPass();
	void OnAllPass();
	void ResetAllPass();

private:
	kEngine* engine_{};

	/// RenderCommandの設定
	RenderCommand noiseCommand_;
	RenderCommand ColorGradingCommand_;
	RenderCommand BlurCommand_;
	RenderCommand VignetteCommand_;

	/// passのHandle
	int noisePassHandle_ = -1;
	int ColorGradingPassHandle_ = -1;
	int BlurPassHandle_ = -1;
	int VignettePassHandle_ = -1;
};