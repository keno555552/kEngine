#include "PostProcessRunner.h"
#include "drawEngine/drawEngine.h"

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
