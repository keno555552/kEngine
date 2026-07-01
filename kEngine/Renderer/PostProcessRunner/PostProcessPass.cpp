#include "PostProcessPass.h"
#include "kEngine/Renderer/drawEngine/DrawEngine.h"

/// ==================== RunPass =================== ///

void PostProcessPass::FinalCopy(DrawEngine* engine) {
	engine->DrawRenderCopy();
}

void PostProcessPass::RunCopy(
	DrawEngine* engine) {
	engine->DrawRenderCopy();
}

void PostProcessPass::RunColorGrading(DrawEngine* engine, RenderCommandGPU command) {
	engine->DrawColorGrading(command);
}

void PostProcessPass::RunVignette(DrawEngine* engine, RenderCommandGPU command) {
	engine->DrawVignette(command);
}

void PostProcessPass::RunBlur(DrawEngine* engine, RenderCommandGPU command) {
	engine->DrawBlur(command);
}

void PostProcessPass::RunOutline(DrawEngine* engine, RenderCommandGPU command) {
	engine->DrawOutline(command);
}

void PostProcessPass::RunOutlinePrewittDepth(DrawEngine* engine, RenderCommandGPU command) {
	engine->DrawOutlinePrewittDepth(command);
}

void PostProcessPass::RunDissolve(DrawEngine* engine, RenderCommandGPU command, int dissolveTextureIndex) {
	engine->DrawDissolve(command, dissolveTextureIndex);
}

void PostProcessPass::RunNoise(DrawEngine* engine, RenderCommandGPU command) {
	engine->DrawNoise(command);
}


/// ======================== PassesRun ======================== ///

/// CopyPass
CopyPass::CopyPass(RenderCommandGPU commandGPU)
{ passName_ = "CopyPass"; }
void CopyPass::Run(DrawEngine* engine) {
	RunCopy(engine);
}

/// ColorGradingPass
ColorGradingPass::ColorGradingPass(RenderCommandGPU commandGPU):
	commandGPU_(commandGPU) 
{ passName_ = "ColorGradingPass"; }
void ColorGradingPass::Run(DrawEngine* engine) {
	RunColorGrading(engine, commandGPU_);
}

/// BlurPass
BlurPass::BlurPass(RenderCommandGPU commandGPU):
	commandGPU_(commandGPU) 
{ passName_ = "BlurPass"; }
void BlurPass::Run(DrawEngine* engine) {
	RunBlur(engine, commandGPU_);
}

/// VignettePass
VignettePass::VignettePass(RenderCommandGPU commandGPU):
	commandGPU_(commandGPU) 
{ passName_ = "VignettePass"; }
void VignettePass::Run(DrawEngine* engine) {
	RunVignette(engine, commandGPU_);
}

/// OutlinePass
OutlinePass::OutlinePass(RenderCommandGPU commandGPU) :
	commandGPU_(commandGPU) 
{ passName_ = "OutlinePass"; }
void OutlinePass::Run(DrawEngine* engine) {
	RunOutline(engine, commandGPU_);
}

/// OutlinePrewittDepthPass
OutlinePrewittDepthPass::OutlinePrewittDepthPass(RenderCommandGPU commandGPU) :
	commandGPU_(commandGPU) 
{ passName_ = "OutlinePrewittDepthPass"; }
void OutlinePrewittDepthPass::Run(DrawEngine* engine) {
	RunOutlinePrewittDepth(engine, commandGPU_);
}

/// DissolvePass
DissolvePass::DissolvePass(RenderCommandGPU commandGPU, int dissolveTextureIndex) :
	commandGPU_(commandGPU)	, 
	dissolveTextureIndex_(dissolveTextureIndex)
{ passName_ = "DissolvePass"; }
void DissolvePass::Run(DrawEngine* engine) {
	RunDissolve(engine, commandGPU_, dissolveTextureIndex_);
}

/// NoisePass
NoisePass::NoisePass(RenderCommandGPU commandGPU) :
	commandGPU_(commandGPU) 
{ passName_ = "NoisePass"; }
void NoisePass::Run(DrawEngine* engine) {
	RunNoise(engine, commandGPU_);
}



//case PostProcessType::None:
//	drawEngine->DrawRenderCopy();
//	break;
//case PostProcessType::ColorGrading:
//	drawEngine->DrawColorGrading();
//	break;
//case PostProcessType::Vignette:
//	drawEngine->DrawVignette();
//	break;
//case PostProcessType::Blur:
//	drawEngine->DrawBlur();
//	break;
//case PostProcessType::Outline:
//	drawEngine->DrawOutline();
//	break;
//case PostProcessType::OutlinePrewittDepth:
//	drawEngine->DrawOutlinePrewittDepth();
//	break;
//case PostProcessType::Dissolve:
//	drawEngine->DrawDissolve();
//	break;
//case PostProcessType::Noise:
//	drawEngine->DrawNoise();

