#include "EffectManager.h"
#include "kEngine.h"

void EffectManager::Initialize(kEngine* system) {
	system_ = system;
	particleManager_ = std::make_unique<ParticleManager>(system);
}

void EffectManager::Finalize() {}


void EffectManager::Update() {

	particleManager_->Update();

}

void EffectManager::Draw() {
	particleManager_->Draw();
}

int EffectManager::AddEffect(const EffectData& data) {

	/// まずは同じEffectDataが存在するかを確認する
	int handle = CheckHaveData(data);
	/// もし存在する場合はそのハンドルを返す
	if (handle != -1) return handle;

	/// ない場合新しいEffectInstanceを作る
	EffectInstance newInstance;
	newInstance.version = 1;
	newInstance.data = &data;

	effectInstances_[counter_] = newInstance;
	effectHandles_.push_back(counter_);
	return counter_++;
}

std::string EffectManager::UpdateEffect(int handle, EffectData& data) {
	///　まずはそのハンドルが存在するかを確認する
	if (std::find(effectHandles_.begin(), effectHandles_.end(), handle) == effectHandles_.end()) {
		return "Update Effect:: Handle not found";
	}

	/// もし存在する場合、EffectInstanceの内容を更新する
	effectInstances_[handle].data = &data;

	///《《《《《《《《《《《《《《《《《《《《《《《《《《《《《《《《《

	return "Update Effect:: Effect updated successfully";
}

void EffectManager::ClearEffect(int handle) {}

void EffectManager::ResetEffect(int handle) {}

void EffectManager::RemoveEffect(int handle) {}


/// Dataが存在するかどうかの関数
int EffectManager::CheckHaveData(const EffectData& data) {

	/// まずは同じEffectDataを探す
	for (auto& pair : effectInstances_) {
		int handle = pair.first;
		const EffectInstance& inst = pair.second;

		/// 見つかったらそのハンドルを返す
		if (inst.data == &data) {
			return handle;
		}
	}
	/// 見つからなかった場合は-1を返す
	return -1;
}
