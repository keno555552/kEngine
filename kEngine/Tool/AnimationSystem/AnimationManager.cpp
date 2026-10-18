#include "AnimationManager.h"
#include "kEngine.h"
#include "externals/nlohmann/json.hpp"
#include "Resource/ResourceManager.h"
#include <Camera/Camera.h>

AnimationManager::AnimationManager(kEngine* system) {
	system_ = system;
}

AnimationManager::~AnimationManager() {
}

std::vector<int> AnimationManager::LoadAnimation(const std::string& filePath) {

	/// まずリストを作る
	std::vector<int> result;

	/// Listに元々あるかどうか確認する,あったらそれをリストに作って返す
	for (int i = 0; i < animationDataList_.size(); ++i) {
		if (animationDataList_[i].filePath == filePath) {
			result.push_back(i);
		}
	}
	if (!result.empty()) return result;


	/// ResourceManagerにローディング/ハンドルを求む
	int modelDataHandle = system_->GetResourceManager()->ReadFile(filePath);
	std::weak_ptr<ModelData> modelData = ResourceManager::GetInstance()->GetModelData(modelDataHandle);

	for (int i = 0; i < modelData.lock()->animationList.size(); ++i) {
		AnimationClip clip{};
		clip.modelDataHandle = modelDataHandle;
		clip.modelDataPtr = modelData;
		clip.animationIndex = i;
		clip.filePath = filePath;
		animationDataList_.push_back(clip);
		result.push_back(int(animationDataList_.size() - 1));
	}

	if (result.empty()) {
		Logger::Log("[kEngine] AM :: Unknow Error.");
	}

	///ハンドルを返す
	return result;
}

void AnimationManager::UnitSetTime(int unitHandle, float time) {

	bool retFlag = CheckHaveHandle(unitHandle);
	if (!retFlag) return;

	unitList_[unitHandle]->SetTime(time);
}

void AnimationManager::ApplySkinning() {

	if (!wellList_ || !skinningDataToPaletteIndexMap_) {
		return; // 尚未初始化
	}

	for (auto& [unitHandle, unit] : unitList_) {

		Object* obj = unit->GetInstanceObject();
		if (!obj) continue;

		std::weak_ptr<ModelData> objData = unit->GetModelData();
		if (objData.expired()) continue;

		// 沒有 skeleton 就不用 skinning
		if (!unit->GetHaveSkeleton()) continue;

		Skeleton* skeleton = unit->GetInstanceSkeleton();
		if (!skeleton) continue;

		auto& joints = skeleton->jointList;
		int jointCount = (int)joints.size();

		// 每個 ObjectPart 都有自己的 wellHandle
		for (auto& part : obj->objectParts_) {

			int handle = part.GetWellHandle();
			if (handle < 0) continue;

			// 1. 用 handle 找 paletteIndex
			auto it = skinningDataToPaletteIndexMap_->find(handle);
			if (it == skinningDataToPaletteIndexMap_->end()) continue;

			int paletteIndex = it->second;

			// 2. 用 paletteIndex 找 WellForGPU
			WellForGPU* palette = (*wellList_)[paletteIndex]->GetInstanceBufferPtr();
			if (!palette) continue;

			// 3. 寫入骨骼矩陣（使用 AnimationUnit 的 finalMatrix）
			for (int j = 0; j < jointCount; ++j) {

				Matrix4x4& finalMat = joints[j].finalMatrix;

				palette[j].skeletonSpaceMatrix = finalMat;
				palette[j].skeletonSpaceInverseTransposeMatrix =
					finalMat.Inverse().Transpose();
			}
		}
	}
}

int AnimationManager::TakeControlObject(int animationHandle, Object* object) {

	/// ハンドルが存在するか確認する
	if (!CheckHaveHandle(animationHandle)) {
		return -1;
	}

	/// nullcheck
	if (!object) {
		Logger::Log("[kError] AM::UnitSetTime() Invalid animation unit handle.");
		return -1;
	}

	/// animationHandleからアニメーションデータを読み込む
	auto& ani = animationDataList_[animationHandle];

	/// モデルデータが有効か確認する
	auto model = ani.modelDataPtr.lock();
	if (!model) {
		Logger::Log("[kError] AM::UnitSetTime() ModelData expired.");
		return -1;
	}

	/// Unitを作成して、アニメーションデータを読み込む
	unitList_[unitHandleCounter_] = std::make_unique<AnimationUnit>(system_);
	unitList_[unitHandleCounter_]->ReadAnimationData(model, ani.animationIndex);
	unitList_[unitHandleCounter_]->TakeControlObject(object);

	/// objectとUnitのHandleを紐づける
	objectToUnitHandleList_[object] = unitHandleCounter_;
	return unitHandleCounter_++;/// Unitのハンドルを返す,そしてカウンター＋1
}

int AnimationManager::GetControlObjectPartHandle(ObjectData* object) {

	/// nullcheck
	if (!object)return -1;

	/// objectがUnitに紐づいているか確認する
	auto it = objectToUnitHandleList_.find(object);
	if (it == objectToUnitHandleList_.end()) {
		return -1;
	}
	/// 紐づいているUnitのHandleを返す
	return it->second;
}

Object* AnimationManager::GetInstanceObjectByUnitHandle(int unitHandle) {
	if (unitList_.contains(unitHandle)) {
		return unitList_[unitHandle]->GetInstanceObject();
	}
	Logger::Log("[kError] AM::GetInstanceObjectByUnitHandle() Invalid animation unit handle.");
	return nullptr;
}

void AnimationManager::Update() {
	for (auto& [handle, unit] : unitList_) {
		unit->Update();
	}

	ApplySkinning();
}

bool AnimationManager::CheckHaveHandle(int animHandle) {

	/// ハンドルが存在するか確認する
	if (animHandle < 0 ||
		animHandle >= animationDataList_.size()) {
		Logger::Log("[kError] AM::CheckHaveHandle() Invalid animation handle.");
		return false;
	}

	/// すでに読み込んでいるファイルならばハンドルを返す
	if (animationDataList_[animHandle].modelDataPtr.expired()) {
		Logger::Log("[kError] AM::CheckHaveHandle() ModelData expired.");
		return false;
	}
	return true;
}