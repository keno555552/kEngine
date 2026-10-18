#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "Object/Object.h"
#include "AnimationUnit.h"
#include "Renderer/Resource/InstanceBuffer.h"
#include "Data/Render/GPUData/WellForGPU.h"

class kEngine;
class AnimationManager
{
public:

	AnimationManager(kEngine* system);
	~AnimationManager();

	void SetWellList(std::vector<std::unique_ptr<InstanceBuffer<WellForGPU>>>* wellList) { wellList_ = wellList; }
	void SetSkinningToPaletteIndexMap(std::map<int, int>* skinningToPaletteIndexMap) { skinningDataToPaletteIndexMap_ = skinningToPaletteIndexMap; }

	/// ====================== アニメーションシステムの基本的な流れ ========================///
	std::vector<int> LoadAnimation(const std::string& filePath);

	/// 時間を調整する
	void UnitSetTime(int unitHandle, float time);

	void ApplySkinning();

	/// ======================= 操作関連 ========================///

	int TakeControlObject(int animationHandle, Object* object);

	int GetControlObjectPartHandle(ObjectData* object);

	Object* GetInstanceObjectByUnitHandle(int unitHandle);

	void Update();

private:

	/// ============= システム関連 ============== ///
	kEngine* system_{};

	/// ============= アニメーション関連 ============== ///
	struct AnimationClip {
		int modelDataHandle{};
		std::weak_ptr<ModelData> modelDataPtr;
		int animationIndex{};
		std::string filePath;
	};

	// アニメーションデータのリスト
	std::vector<AnimationClip> animationDataList_{};

	// Unitのリスト
	std::map<int, std::unique_ptr<AnimationUnit>> unitList_{};
	std::unordered_map<ObjectData*, int> objectToUnitHandleList_{};
	int unitHandleCounter_{};

	// Skinning Wellの資料リスト
	std::vector<std::unique_ptr<InstanceBuffer<WellForGPU>>>* wellList_{};
	std::map<int, int>* skinningDataToPaletteIndexMap_{};

private:
	/// Unitのハンドルが存在するか確認する,存在しないならばエラーを出してfalseを返す
	bool CheckHaveHandle(int animHandle);
};

