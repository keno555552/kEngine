#pragma once
#include "Data/Render/GPUData/RenderCommandGPU.h"

class DrawEngine;
class PostProcessPass
{
public:
	virtual void Run(DrawEngine* engine) = 0;
	std::string GetPassName() { return passName_; }

public:
	/// 最終的にBackBufferにコピーするためのPass
	static void FinalCopy(DrawEngine* engine);

protected:
	void RunCopy(
		DrawEngine* engine);

	void RunColorGrading(
		DrawEngine* engine,
		RenderCommandGPU command);

	void RunVignette(
		DrawEngine* engine,
		RenderCommandGPU command);

	void RunBlur(
		DrawEngine* engine,
		RenderCommandGPU command);

	void RunOutline(
		DrawEngine* engine,
		RenderCommandGPU command);

	void RunOutlinePrewittDepth(
		DrawEngine* engine,
		RenderCommandGPU command);

	void RunDissolve(
		DrawEngine* engine,
		RenderCommandGPU command,
		int dissolveTextureIndex);

	void RunNoise(
		DrawEngine* engine,
		RenderCommandGPU command);

	std::string passName_{ "defaultPass" };
};

/// ==================  調試中　================== ///
class CopyPass : public PostProcessPass {
public:
	CopyPass(RenderCommandGPU commandGPU);
	void Run(DrawEngine* engine)override;
};

class ColorGradingPass : public PostProcessPass {
public:
	ColorGradingPass(RenderCommandGPU commandGPU);
	void Run(DrawEngine* engine)override;
public:
	RenderCommandGPU commandGPU_{};
};

class VignettePass : public PostProcessPass {
public:
	VignettePass(RenderCommandGPU commandGPU);
	void Run(DrawEngine* engine)override;
public:
	RenderCommandGPU commandGPU_{};
};

class BlurPass : public PostProcessPass {
public:
	BlurPass(RenderCommandGPU commandGPU);
	void Run(DrawEngine* engine)override;
public:
	RenderCommandGPU commandGPU_{};
};

class OutlinePass : public PostProcessPass {
public:
	OutlinePass(RenderCommandGPU commandGPU);
	void Run(DrawEngine* engine)override;
public:
	RenderCommandGPU commandGPU_{};
};

class OutlinePrewittDepthPass : public PostProcessPass {
public:
	OutlinePrewittDepthPass(RenderCommandGPU commandGPU);
	void Run(DrawEngine* engine)override;
public:
	RenderCommandGPU commandGPU_{};
};

class DissolvePass : public PostProcessPass {
public:
	DissolvePass(RenderCommandGPU commandGPU, int dissolveTextureIndex);
	void Run(DrawEngine* engine)override;
public:
	RenderCommandGPU commandGPU_{};
	int dissolveTextureIndex_{};
};

class NoisePass : public PostProcessPass {
public:
	NoisePass(RenderCommandGPU commandGPU);
	void Run(DrawEngine* engine)override;
public:
	RenderCommandGPU commandGPU_{};
};