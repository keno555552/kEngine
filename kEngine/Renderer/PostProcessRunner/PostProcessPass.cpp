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

void PostProcessPass::RunBlur(DrawEngine* engine, RenderCommandGPU command, KernelDataGPU kernelData) {
	engine->DrawBlur(command, kernelData);
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
void CopyPass::Run(DrawEngine* engine) { RunCopy(engine); }

/// ====================  ColorGradingPass
ColorGradingPass::ColorGradingPass(RenderCommandGPU commandGPU):
	commandGPU_(commandGPU) 
{ passName_ = "ColorGradingPass"; }
void ColorGradingPass::Run(DrawEngine* engine) { RunColorGrading(engine, commandGPU_); }
void ColorGradingPass::SetCommandGPU(RenderCommandGPU commandGPU) { commandGPU_ = commandGPU; }

/// ====================  VignettePass
VignettePass::VignettePass(RenderCommandGPU commandGPU):
	commandGPU_(commandGPU) 
{ passName_ = "VignettePass"; }
void VignettePass::Run(DrawEngine* engine) { RunVignette(engine, commandGPU_); }
void VignettePass::SetCommandGPU(RenderCommandGPU commandGPU) { commandGPU_ = commandGPU; }

/// ====================  BlurPass
BlurPass::BlurPass(RenderCommandGPU commandGPU):
	commandGPU_(commandGPU) 
{ passName_ = "BlurPass"; }
void BlurPass::Run(DrawEngine* engine) { RunBlur(engine, commandGPU_, kernelData_); }
void BlurPass::SetCommandGPU(RenderCommandGPU commandGPU) { commandGPU_ = commandGPU; }
void BlurPass::SetKernel(KernelDataGPU kernelData)		  { kernelData_ = kernelData; }

/// ====================  OutlinePass
OutlinePass::OutlinePass(RenderCommandGPU commandGPU) :
	commandGPU_(commandGPU) 
{ passName_ = "OutlinePass"; }
void OutlinePass::Run(DrawEngine* engine) { RunOutline(engine, commandGPU_); }
void OutlinePass::SetCommandGPU(RenderCommandGPU commandGPU) { commandGPU_ = commandGPU; }
void OutlinePass::SetKernel(KernelDataGPU kernelData)        { kernelData_ = kernelData; }


/// ====================  OutlinePrewittDepthPass
OutlinePrewittDepthPass::OutlinePrewittDepthPass(RenderCommandGPU commandGPU) :
	commandGPU_(commandGPU) 
{ passName_ = "OutlinePrewittDepthPass"; }
void OutlinePrewittDepthPass::Run(DrawEngine* engine) { RunOutlinePrewittDepth(engine, commandGPU_); }
void OutlinePrewittDepthPass::SetCommandGPU(RenderCommandGPU commandGPU) { commandGPU_ = commandGPU; }

/// ====================  DissolvePass
DissolvePass::DissolvePass(RenderCommandGPU commandGPU, int dissolveTextureIndex) :
	commandGPU_(commandGPU)	, 
	dissolveTextureIndex_(dissolveTextureIndex)
{ passName_ = "DissolvePass"; }
void DissolvePass::Run(DrawEngine* engine) { RunDissolve(engine, commandGPU_, dissolveTextureIndex_); }
void DissolvePass::SetCommandGPU(RenderCommandGPU commandGPU) { commandGPU_ = commandGPU; }

/// ====================  NoisePass
NoisePass::NoisePass(RenderCommandGPU commandGPU) :
	commandGPU_(commandGPU) 
{ passName_ = "NoisePass"; }
void NoisePass::Run(DrawEngine* engine) { RunNoise(engine, commandGPU_); }
void NoisePass::SetCommandGPU(RenderCommandGPU commandGPU) { commandGPU_ = commandGPU; }




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

