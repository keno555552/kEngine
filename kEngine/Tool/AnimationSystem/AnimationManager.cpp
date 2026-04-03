#include "AnimationManager.h"
#include "kEngine.h"
#include "externals/nlohmann/json.hpp"
#include <Camera/Camera.h>

AnimationManager::AnimationManager(kEngine* system) {
	system_ = system;
}

AnimationManager::~AnimationManager() {
}

Animation AnimationManager::LoadAnimationData(const std::string& filePath) {
	size_t dotPos = filePath.rfind('.');

	/// ファイルが変
	if (dotPos == std::string::npos) {
		Logger::Log("[kError] AE :: File path does not contain an extension.");
		return {};
	}

	/// ファイル拡張子による処理分岐
	std::string fileExt = filePath.substr(dotPos);

	/// AnimationDataを読み込むための構造体を作る
	Animation result;

	if (fileExt == ".json") {
		//result = LoadAnimationDataFromJson(filePath);
		Logger::Log("[kError] AE :: json reader not Ready.");
		return {};
	}
	if (fileExt == ".gltf") {
		result = LoadAnimationDataFromGltf(filePath);
		return result;
	}

	Logger::Log("[kError] AE :: File not support.");
	return {};
}

int AnimationManager::LoadAnimation(const std::string& filePath) {

	/// すでに読み込んでいるファイルならばハンドルを返す
	if (animationHandleList_.contains(filePath)) {
		return animationHandleList_[filePath];
	}

	/// 読み込んでいないファイルならば読み込む
	Animation animationData = LoadAnimationData(filePath);
	if (animationData.nodeList.size() == 0) {
		Logger::Log("[kError] AE :: Failed to load animation data.");
		return -1;
	}

	/// 読み込んだアニメーションデータを保存する
	std::shared_ptr<Animation> animationDataPtr = std::make_shared<Animation>(animationData);
	animationDataList_.push_back(animationDataPtr);
	int index = (int)animationDataList_.size() - 1;

	/// HandleListに保存する
	animationHandleList_[filePath] = index;

	/// Unitを作って、アニメーションデータを読み込む
	unitList_[index] = std::make_unique<AnimationUnit>(system_);
	unitList_[index]->ReadAnimationData(animationDataPtr);

	///ハンドルを返す
	return index;
}

void AnimationManager::UnitSetTime(int unitHandle, float time) {

	bool retFlag = CheckHaveHandle(unitHandle);
	if (!retFlag) return;

	unitList_[unitHandle]->SetTime(time);
}


void AnimationManager::TakeControlObject(int unitHandle, Object* object) {

	if (!CheckHaveHandle(unitHandle)) {
		return;
	}

	if (!object) {
		Logger::Log("[kError] AS :: Invalid animation unit handle.");
		return;
	}

	unitList_[unitHandle]->TakeControlObject(object);
}

void AnimationManager::Update() {
	for (auto& [handle, unit] : unitList_) {
		unit->Update();
	}
}

Animation AnimationManager::LoadAnimationDataFromGltf(const std::string& filePath) {

	std::ifstream input(filePath);
	if (!input.is_open()) {
		throw std::runtime_error("Failed to open file: " + filePath);
	}

	/// ファイルを読み込む
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);

	/// アニメーションデータがない
	if (scene->mNumAnimations == 0) {
		Logger::Log("[kError] AE :: This file does not contain animation data.");
		return{};
	}

	/// 受け皿を作る
	Animation result;
	/// 最初のアニメーションを読み込む
	aiAnimation* animationAssimp = scene->mAnimations[0];

	/// アニメーションの長さを秒数で計算して保存
	result.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

	/// assimpで一つずつのNodeのAnimationをchannelと呼んでいるのてchannelを回してNodeAnimationの情報をお取ってくる
	for (int channelIndex = 0; channelIndex < (int)animationAssimp->mNumChannels; channelIndex++) {
		AnimationNodeData nodeData{};
		aiNodeAnim* nodeAnim = animationAssimp->mChannels[channelIndex];
		nodeData.name = nodeAnim->mNodeName.C_Str();
		nodeData.animationNodeID = channelIndex;
		/// スケールのKeyFrameを保存
		for (int keyIndex = 0; keyIndex < (int)nodeAnim->mNumScalingKeys; keyIndex++) {
			KeyFrameVector3 keyFrame;
			keyFrame.time_ = float(nodeAnim->mScalingKeys[keyIndex].mTime / animationAssimp->mTicksPerSecond);
			keyFrame.value_ = { nodeAnim->mScalingKeys[keyIndex].mValue.x,
								nodeAnim->mScalingKeys[keyIndex].mValue.y,
								nodeAnim->mScalingKeys[keyIndex].mValue.z, };
			keyFrame.animationType_ = AnimationType::LINEARITY;
			nodeData.scaleList.push_back(keyFrame);
		}
		/// 回転のKeyFrameを保存
		for (int keyIndex = 0; keyIndex < (int)nodeAnim->mNumRotationKeys; keyIndex++) {
			KeyFrameQuaternion keyFrame;
			keyFrame.time_ = float(nodeAnim->mRotationKeys[keyIndex].mTime / animationAssimp->mTicksPerSecond);
			Quaternion quatValue = { nodeAnim->mRotationKeys[keyIndex].mValue.x,
									 nodeAnim->mRotationKeys[keyIndex].mValue.y,
									 nodeAnim->mRotationKeys[keyIndex].mValue.z,
									 nodeAnim->mRotationKeys[keyIndex].mValue.w, };
			keyFrame.value_ = quatValue;
			keyFrame.animationType_ = AnimationType::LINEARITY;
			nodeData.rotateList.push_back(keyFrame);
		};
		/// 平行移動のKeyFrameを保存
		for (int keyIndex = 0; keyIndex < (int)nodeAnim->mNumPositionKeys; keyIndex++) {
			KeyFrameVector3 keyFrame;
			keyFrame.time_ = float(nodeAnim->mPositionKeys[keyIndex].mTime / animationAssimp->mTicksPerSecond);
			keyFrame.value_ = { nodeAnim->mPositionKeys[keyIndex].mValue.x,
								nodeAnim->mPositionKeys[keyIndex].mValue.y,
								nodeAnim->mPositionKeys[keyIndex].mValue.z, };
			keyFrame.animationType_ = AnimationType::LINEARITY;
			nodeData.translationList.push_back(keyFrame);
		}
		result.nodeList.push_back(nodeData);
	}

	return result;
}

Animation AnimationManager::LoadAnimationDataFromJson(const std::string& filePath) {
	//std::ifstream input(filePath);
	//if (!input.is_open()) {
	//	throw std::runtime_error("Failed to open file: " + filePath);
	//}
	//
	//nlohmann::json file;
	//input >> file;  // 讀取並解析 JSON
	//
	//// 基本資訊
	//
	//Animation result;
	//result.nodeList[0].animationNodeID = file["AnimationID"].get<int>();
	//
	//// KeyFrameList
	//for (const auto& kfJson : file["KeyFrameList"]) {
	//	KeyFrame kf;
	//	kf.index_ = kfJson["Index"].get<int>();
	//	kf.time_ = kfJson["Time"].get<float>();
	//	kf.animationType_ = static_cast<AnimationType>(kfJson["AnimationType"].get<int>());
	//	kf.easeRate_ = kfJson["EaseRate"].get<float>();
	//
	//	// MainPosition
	//	auto mp = kfJson["TransformData"]["MainPosition"];
	//	kf.transformData.mainPosition.transform.translate.x = mp["TranslateX"].get<float>();
	//	kf.transformData.mainPosition.transform.translate.y = mp["TranslateY"].get<float>();
	//	kf.transformData.mainPosition.transform.translate.z = mp["TranslateZ"].get<float>();
	//	kf.transformData.mainPosition.transform.rotate.x = mp["RotateX"].get<float>();
	//	kf.transformData.mainPosition.transform.rotate.y = mp["RotateY"].get<float>();
	//	kf.transformData.mainPosition.transform.rotate.z = mp["RotateZ"].get<float>();
	//	kf.transformData.mainPosition.transform.scale.x = mp["ScaleX"].get<float>();
	//	kf.transformData.mainPosition.transform.scale.y = mp["ScaleY"].get<float>();
	//	kf.transformData.mainPosition.transform.scale.z = mp["ScaleZ"].get<float>();
	//
	//	// ObjectParts
	//	for (const auto& partJson : kfJson["TransformData"]["ObjectPart"]) {
	//		ObjectPart part;
	//		part.transform.translate.x = partJson["TranslateX"].get<float>();
	//		part.transform.translate.y = partJson["TranslateY"].get<float>();
	//		part.transform.translate.z = partJson["TranslateZ"].get<float>();
	//		part.transform.rotate.x = partJson["RotateX"].get<float>();
	//		part.transform.rotate.y = partJson["RotateY"].get<float>();
	//		part.transform.rotate.z = partJson["RotateZ"].get<float>();
	//		part.transform.scale.x = partJson["ScaleX"].get<float>();
	//		part.transform.scale.y = partJson["ScaleY"].get<float>();
	//		part.transform.scale.z = partJson["ScaleZ"].get<float>();
	//
	//		kf.transformData.objectParts_.push_back(part);
	//	}
	//
	//	animationList->keyList.push_back(kf);
	//}
	//keyFrameList_.clear();
	//for (auto& kf : animationList->keyList) {
	//	KeyFrame newKf;
	//	newKf.index_ = kf.index_;
	//	newKf.time_ = kf.time_;
	//	newKf.animationType_ = kf.animationType_;
	//	newKf.easeRate_ = kf.easeRate_;
	//	newKf.transformData = kf.transformData;
	//	newKf.hitBox_ = {
	//		{ startPos_.x + ((EndPos_.x - startPos_.x) / mainTimer_->maxTime_) * kf.time_,
	//		  (float)config::GetClientHeight() - 10.0f + 5.0f },
	//		10.0f, 10.0f
	//	};
	//	keyFrameList_.push_back(newKf);
	//}
	//
	//ping_->objectParts_.clear();
	//for (const auto& kf : keyFrameList_) {
	//	ping_->CreateDefaultData();
	//	float Leight = EndPos_.x - startPos_.x;
	//	float timeParameter = kf.time_ / mainTimer_->maxTime_;
	//	ping_->objectParts_.back().materialConfig->textureHandle = pingTH_;
	//	ping_->objectParts_.back().transform.translate = {
	//		startPos_.x + Leight * timeParameter,
	//		(float)config::GetClientHeight() - 10.0f,
	//		100.0f
	//	};
	//	ping_->objectParts_.back().anchorPoint = { 5.0f,0.0f };
	//}
	return {};
}


bool AnimationManager::CheckHaveHandle(int unitHandle) {

	/// すでに読み込んでいるファイルならばハンドルを返す
	if (!unitList_.contains(unitHandle)) {
		Logger::Log("[kError] AS :: No such unit.");
		return false;
	}
	return true;
}