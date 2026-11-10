#pragma once
#include "Data/Render/CPUData/RenderCommand.h"

class kEngine;
class PPP_GlitchScreen {

public:
	PPP_GlitchScreen(kEngine* engine);

	RenderCommand* GetNoiseCommand() { return &noiseCommand_; }
	RenderCommand* GetColorGradingCommand() { return &colorGradingCommand_; }
	RenderCommand* GetBlurCommand() { return &blurCommand_; }
	RenderCommand* GetVignetteCommand() { return &vignetteCommand_; }

	void UpdateNoiseCommand(RenderCommand command) { noiseCommand_ = command; }
	void Update();

	void OffAllPass();
	void OnAllPass();
	void ResetAllPass();

private:
	kEngine* engine_{};

	/// RenderCommandの設定
	RenderCommand noiseCommand_;
	RenderCommand colorGradingCommand_;
	RenderCommand blurCommand_;
	RenderCommand vignetteCommand_;

	/// passのHandle
	int noisePassHandle_ = -1;
	int ColorGradingPassHandle_ = -1;
	int BlurPassHandle_ = -1;
	int VignettePassHandle_ = -1;
};