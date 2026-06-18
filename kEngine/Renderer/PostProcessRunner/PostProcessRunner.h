#pragma once
#include <vector>
#include <map>
#include "Data/Render/Types/PostProcessType.h"
#include "Data/Render/CPUData/RenderCommand.h"
#include "Data/Render/GPUData/RenderCommandGPU.h"
#include "Data/Render/GPUData/BlurDataGPU.h"

using PostProcessLayer = int;

class DrawEngine;
class PostProcessRunner {
public:
	PostProcessRunner();

	void SetChain(const std::vector<PostProcessType>& chain);

	void Execute(DrawEngine* engine);
	RenderCommand& GetRenderCommand() { return renderCommand_; }
	void ChangeRenderCommand(const RenderCommand& command);
	void SetRenderCommand(DrawEngine* engine);

	/// 描画システムを設定用関数

	void SetBlurlData(const RenderCommand& renderCommand, int instanceIndex);
	void SetOutlinelData(const RenderCommand& renderCommand, int instanceIndex);
	void SetInstanceListBlurData(KernelDataGPU* instancingListBlurData) { instancingListBlurData_ = instancingListBlurData; }
	KernelDataGPU* GetInstanceListBlurData() { return instancingListBlurData_; }

private:
	std::vector<PostProcessType> chain_;

	void RunPass(DrawEngine* drawEngine, PostProcessType type);
	RenderCommand renderCommand_{};
	std::array<RenderCommandGPU, 5> renderCommandGPUPerLayer_{};

	KernelDataGPU* instancingListBlurData_ = nullptr;
	int instanceCounterBlurData_ = 0;

};
