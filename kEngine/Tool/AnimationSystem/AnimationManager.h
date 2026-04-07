#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "Object/Object.h"
#include "AnimationUnit.h"

class kEngine;
class AnimationManager
{
public:

	AnimationManager(kEngine* system);
	~AnimationManager();
	
	/// ====================== 独立読み込関連 ======================== ///
	/// アニメーションデータを読み込む
	static Animation LoadAnimationData(const std::string& filePath);

	/// ====================== アニメーションシステムの基本的な流れ ========================///
	int LoadAnimation(const std::string& filePath);

	/// 時間を調整する
	void UnitSetTime(int unitHandle, float time);


	/// ======================= 操作関連 ========================///

	void TakeControlObject(int unitHandle, Object* object);

	int GetControlObjectPartHandle(ObjectData* object);

	Object* GetInstanceObjectByUnitHandle(int unitHandle);

	void Update();

private:

	/// ============= システム関連 ============== ///
	kEngine* system_{};

	/// ============= アニメーション関連 ============== ///
	// アニメーションデータのリスト
	std::vector<std::shared_ptr<Animation>> animationDataList_{};
	std::map<std::string, int> animationHandleList_{};

	// Unitのリスト
	std::map<int, std::unique_ptr<AnimationUnit>> unitList_{};
	std::unordered_map<ObjectData*, int> objectToUnitHandleList_{};

private:
	 
	/// ====================== 読み込関連 ======================== ///
	static Animation LoadAnimationDataFromGltf(const std::string& filePath);
	static Animation LoadAnimationDataFromJson(const std::string& filePath);


	// Unitのハンドルが存在するか確認する,存在しないならばエラーを出してfalseを返す
	bool CheckHaveHandle(int unitHandle);
};

