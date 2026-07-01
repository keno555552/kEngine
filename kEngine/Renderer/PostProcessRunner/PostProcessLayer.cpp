#include "PostProcessLayer.h"
#include "drawEngine/drawEngine.h"
#include <utility>

void PostProcessLayer::AddPass(std::unique_ptr<PostProcessPass> pass) {
	passList_.push_back(std::move(pass));
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


