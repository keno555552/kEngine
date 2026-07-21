#include "PostProcessRunner.h"
#include "drawEngine/drawEngine.h"

PostProcessRunner::PostProcessRunner() {
	/// 後処理のレイヤーを設定し、初期化する
	for (int i = 0; i < config::GetPostProcessLayer(); ++i) {
		layerList_.push_back(std::make_unique<PostProcessLayer>());
	}
	renderTarget_.Init();
}

/// ==================== SetPass ==================== ///

int PostProcessRunner::SetPassCopy(int layer, RenderCommand command) {
	/// layerが範囲外の場合、エラーを出す
	if (LayerCheck(layer)) return -1;

	/// CopyPassを追加する
	RenderCommandGPU commandGPU;
	ConvertRenderCommandToGPU(command, commandGPU);
	return layerList_[layer]->AddPass(std::make_unique<CopyPass>(commandGPU));
}

int PostProcessRunner::SetPassColorGrading(int layer, RenderCommand command) {
	/// layerが範囲外の場合、エラーを出す
	if (LayerCheck(layer)) return -1;

	/// CopyPassを追加する
	RenderCommandGPU commandGPU;
	ConvertRenderCommandToGPU(command, commandGPU);
	return layerList_[layer]->AddPass(std::make_unique<ColorGradingPass>(commandGPU));
}

int PostProcessRunner::SetPassVignette(int layer, RenderCommand command) {
	/// layerが範囲外の場合、エラーを出す
	if (LayerCheck(layer)) return -1;

	/// CopyPassを追加する
	RenderCommandGPU commandGPU;
	ConvertRenderCommandToGPU(command, commandGPU);
	return layerList_[layer]->AddPass(std::make_unique<VignettePass>(commandGPU));
}

int PostProcessRunner::SetPassBlur(int layer, RenderCommand command) {
	/// layerが範囲外の場合、エラーを出す
	if (LayerCheck(layer)) return -1;

	/// CopyPassを追加する
	RenderCommandGPU commandGPU;
	ConvertRenderCommandToGPU(command, commandGPU);
	commandGPU.blurKernelIndex = 0;
	int handle =  layerList_[layer]->AddPass(std::make_unique<BlurPass>(commandGPU));

	/// kernelをGPU形成に転換し設定する
	if (command.blurType != KernelType::NONE) {
		KernelDataGPU kernelData;
		kernelData.ConvertOutlineCommand(command);
		layerList_[layer]->SetPassKernelGPU(handle, kernelData);
	}

	return handle;
}

int PostProcessRunner::SetPassOutline(int layer, RenderCommand command) {
	/// layerが範囲外の場合、エラーを出す
	if (LayerCheck(layer)) return -1;

	/// CopyPassを追加する
	RenderCommandGPU commandGPU;
	ConvertRenderCommandToGPU(command, commandGPU);
	commandGPU.outlineKernelIndex = 0;
	int handle = layerList_[layer]->AddPass(std::make_unique<OutlinePass>(commandGPU));

	/// kernelをGPU形成に転換し設定する
	if (command.outlineType != KernelType::NONE) {
		KernelDataGPU kernelData;
		kernelData.ConvertOutlineCommand(command);
		layerList_[layer]->SetPassKernelGPU(handle, kernelData);
	}

	return handle;
}

int PostProcessRunner::SetPassOutlinePrewittDepth(int layer, RenderCommand command) {
	/// layerが範囲外の場合、エラーを出す
	if (LayerCheck(layer)) return -1;

	/// CopyPassを追加する
	RenderCommandGPU commandGPU;
	ConvertRenderCommandToGPU(command, commandGPU);
	return layerList_[layer]->AddPass(std::make_unique<OutlinePrewittDepthPass>(commandGPU));
}

int PostProcessRunner::SetPassDissolve(int layer, RenderCommand command) {
	/// layerが範囲外の場合、エラーを出す
	if (LayerCheck(layer)) return -1;

	/// CopyPassを追加する
	RenderCommandGPU commandGPU;
	ConvertRenderCommandToGPU(command, commandGPU);
	return layerList_[layer]->AddPass(std::make_unique<DissolvePass>(commandGPU, command.dissolveTextureIndex));
}

int PostProcessRunner::SetPassNoise(int layer, RenderCommand command) {
	/// layerが範囲外の場合、エラーを出す
	if (LayerCheck(layer)) return -1;

	/// CopyPassを追加する
	RenderCommandGPU commandGPU;
	ConvertRenderCommandToGPU(command, commandGPU);
	return layerList_[layer]->AddPass(std::make_unique<NoisePass>(commandGPU));

}

void PostProcessRunner::SetPassCommand(int layer, int passIndex, RenderCommand command) {
	/// layerとpassIndexが範囲外の場合、エラーを出す
	PassCheck(layer, passIndex);

	/// GPUCommandに転換
	RenderCommandGPU commandGPU;
	ConvertRenderCommandToGPU(command, commandGPU);

	/// layerが指定されている場合、該当layerのpassのGPUコマンドを設定する
	layerList_[layer]->SetPassCommandGPU(passIndex, commandGPU);

	/// kernelがある場合、GPUCommandに転換して設定する
	bool needKernel = false;
	if (command.blurType != KernelType::NONE) needKernel = true;
	if (command.outlineType != KernelType::NONE) needKernel = true;
	if (needKernel) {
		KernelDataGPU kernelData;
		kernelData.ConvertOutlineCommand(command);
		layerList_[layer]->SetPassKernelGPU(passIndex, kernelData);
	}
}

void PostProcessRunner::ClearLayer(int layer) {
	/// layerが範囲外の場合、エラーを出す
	if (LayerCheck(layer)) {
		return;
	}
	/// layerが-1の場合、全てのlayerのpassを消す
	if (layer == -1) {
		for (auto& layer : layerList_) {
			layer->ClearPass();
		}
	} else {
		/// layerが指定されている場合、該当layerのpassを消す
		layerList_[layer]->ClearPass();
	}
}

/// ==================== PassListに関係する他の関数 ==================== ///
void PostProcessRunner::ClearPass(int layer, int passIndex) {
	/// layerとpassIndexが範囲外の場合、エラーを出す
	if (PassCheck(layer, passIndex)) {
		return;
	}

	/// layerが-1の場合、このlayerの全てのpassを消す
	if (layer == -1) {
		layerList_[layer]->ClearPass();
	} else {
		/// layerが指定されている場合、該当layerのpassを消す
		layerList_[layer]->ClearPass(passIndex);
	}
}

std::string PostProcessRunner::GetPassName(int layer, int passIndex) {
	/// layerとpassIndexが範囲外の場合、エラーを出す
	if (PassCheck(layer, passIndex)) {
		return "";
	}
	/// layerが指定されている場合、該当layerのpassの名前を取得する
	return layerList_[layer]->GetPassName(passIndex);
}

void PostProcessRunner::Execute(DrawEngine* drawEngine) {
	/// DrawEngine nullptr Check
	DrawEngineCheck(drawEngine);

	/// layerのExecuteを呼び出す
	for (auto& layer : layerList_) {
		layer->Execute(drawEngine);
	}
	PostProcessPass::FinalCopy(drawEngine);
}

void PostProcessRunner::SetPreDraw(DrawEngine* drawEngine) {
	/// DrawEngine nullptr Check
	DrawEngineCheck(drawEngine);

	/// layerのPreDrawを呼び出す
	ResetRenderTargets(drawEngine);
	ClearRenderTargets(drawEngine);
}

void PostProcessRunner::SetRenderTargetsForDraw(DrawEngine* drawEngine) {

	/// DrawEngine nullptr Check
	DrawEngineCheck(drawEngine);

	/// 1. color → PS
	drawEngine->TransitionRenderTarget(
		renderTarget_.inputRT,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	/// 2. depth → SHADER_RESOURCE
	drawEngine->TransitionRenderTarget(
		renderTarget_.outputRT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
}

void PostProcessRunner::SetRenderCommand(DrawEngine* drawEngine, const RenderCommandGPU& command) {
	/// DrawEngine nullptr Check
	DrawEngineCheck(drawEngine);

	drawEngine->commandList_->SetGraphicsRoot32BitConstants(
		1, // RootParameter index（b0,slot[1]）
		sizeof(command) / sizeof(uint32_t),
		&command,
		0);
}

void PostProcessRunner::SetKernelData(const KernelDataGPU& kernelData) {
	instancingListBlurData_[0] = kernelData;
}

PPRenderTexture& PostProcessRunner::GetRenderTarget() {
	return renderTarget_;
}

void PostProcessRunner::ResetRenderTargets(DrawEngine* drawEngine) {
	/// DrawEngine nullptr Check
	DrawEngineCheck(drawEngine);

	/// 1. color → RTV
	drawEngine->TransitionRenderTarget(
		renderTarget_.inputRT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	/// 2. depth → DEPTH_WRITE
	drawEngine->TransitionDepthStencil(
		renderTarget_.inputRT,
		D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void PostProcessRunner::ClearRenderTargets(DrawEngine* drawEngine) {

	/// DrawEngine nullptr Check
	DrawEngineCheck(drawEngine);

	/// Handle取り
	auto rtv = renderTarget_.inputRT.rtvHandleCPU;
	auto dsv = renderTarget_.inputRT.dsvHandleCPU;
	/// inputRenderTargetを設定
	drawEngine->commandList_->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	/// 画面を初期化する
	float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
	drawEngine->commandList_->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	drawEngine->commandList_->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void PostProcessRunner::DrawEngineCheck(DrawEngine* drawEngine) {

	/// DrawEngine nullptr Check,もしnullptrならログを出してassertする
	if (drawEngine == nullptr) {
		Logger::Log("[kEngine]PostProcessRunner::DrawEngineCheck() drawEngine is nullptr");
		assert(false);
	}
}

bool PostProcessRunner::LayerCheck(int layer) {
	if (layerList_.empty()) {
		Logger::Log("[kEngine]PostProcessRunner::LayerCheck() layerList_ is empty, Building Error");
		return true;
	}
	if (layer < 0 || layer >= layerList_.size()) {
		Logger::Log("[kEngine]PostProcessRunner::LayerCheck() layer is out of range");
		return true;
	}
	return false;
}

bool PostProcessRunner::PassCheck(int layer, int passIndex) {
	if (LayerCheck(layer)) {
		return true;
	}
	if (layerList_[layer]->GetPassCount() == 0) {
		Logger::Log("[kEngine]PostProcessRunner::PassCheck() layerList_[layer] is empty");
		return true;
	}
	if (passIndex < 0 || passIndex >= layerList_[layer]->GetPassCount()) {
		Logger::Log("[kEngine]PostProcessRunner::PassCheck() passIndex is out of range");
		return true;
	}
	return false;
}
