#include "PostProcessRunner.h"
#include "drawEngine/drawEngine.h"

PostProcessRunner::PostProcessRunner() {
	for (int i = 0; i < 5; ++i) {
		ConvertRenderCommandToGPU(renderCommand_, renderCommandGPUPerLayer_[i]);
	}
}

void PostProcessRunner::SetChain(const std::vector<PostProcessType>& chain) {
	chain_ = chain;
}

void PostProcessRunner::Execute(DrawEngine* engine) {
	for (auto type : chain_) {
		if (type == PostProcessType::None)continue;
		RunPass(engine, type);
	}
	RunPass(engine, PostProcessType::None);
}

void PostProcessRunner::RunPass(DrawEngine* drawEngine, PostProcessType type) {
	switch (type) {
	case PostProcessType::None:
		drawEngine->DrawRenderCopy();
		break;
	case PostProcessType::ColorGrading:
		drawEngine->DrawColorGrading();
		break;
	case PostProcessType::Vignette:
		drawEngine->DrawVignette();
		break;
	case PostProcessType::Blur:
		drawEngine->DrawBlur();
		break;
	}
}

void PostProcessRunner::ChangeRenderCommand(const RenderCommand& command) {
	renderCommand_ = command;
	ConvertRenderCommandToGPU(renderCommand_, renderCommandGPUPerLayer_[0]);
	SetBlurData(command, 0);

}

void PostProcessRunner::SetRenderCommand(DrawEngine* drawEngine) {
	
	drawEngine->commandList_->SetGraphicsRoot32BitConstants(
		1,  // RootParameter index（b0,slot[1]）
		sizeof(RenderCommandGPU) / sizeof(uint32_t),
		&renderCommandGPUPerLayer_[0],
		0 
	);
}

void PostProcessRunner::SetBlurData(const RenderCommand& renderCommand, int instanceIndex) {
	if (instanceIndex >= 5) {
		Logger::Log("[kEngine]PostProcessRunner::SetBlurData() instanceIndex is out of range");
		return;
	}
	instancingListBlurData_[instanceIndex].ConvertBlurCommand(renderCommand);
}
