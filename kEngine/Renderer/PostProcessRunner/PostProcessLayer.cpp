#include "PostProcessLayer.h"
#include "drawEngine/drawEngine.h"
#include <utility>

int PostProcessLayer::AddPass(std::unique_ptr<PostProcessPass> pass) {
	int handle = static_cast<int>(passList_.size());
	passList_.push_back(std::move(pass));
	return handle;
}

void PostProcessLayer::ClearPass(int passIndex) {

	/// defaultの場合、全部消すと定義する
	if (passIndex == -1) {
		passList_.clear();
		return;
	}

	if (passIndex >= 0 && passIndex < passList_.size()) {
		passList_.erase(passList_.begin() + passIndex);
	}
}

std::string PostProcessLayer::GetPassName(int passIndex) {
	if (passIndex < 0 || passIndex >= passList_.size()) return "";
	return passList_[passIndex]->GetPassName();
}

void PostProcessLayer::Execute(DrawEngine* drawEngine) {

	/// ない場合、何もしない
	if (passList_.empty()) return;

	/// 実行
	for (const auto& pass : passList_) {
		pass->Run(drawEngine);
	}
}

void PostProcessLayer::SetPassCommandGPU(int passIndex, RenderCommandGPU commandGPU) {
	if (passIndex < 0 || passIndex >= passList_.size()) return;
	passList_[passIndex]->SetCommandGPU(commandGPU);
}

void PostProcessLayer::SetPassKernelGPU(int passIndex, KernelDataGPU kernelData) {
	if (passIndex < 0 || passIndex >= passList_.size()) return;
	passList_[passIndex]->SetKernel(kernelData);
}
