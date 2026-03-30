#include "AnimationSystem.h"
#include "externals/nlohmann/json.hpp"
#include <Camera/Camera.h>

AnimationSystem::AnimationSystem(kEngine* system) {
	system_ = system;

	nowTime_ = 0.0f;

	instanceObject_ = std::make_unique<Object>();
	instanceObject_->IntObject(system_);
}

AnimationSystem::~AnimationSystem() {
}

Animation AnimationSystem::LoadAnimationData(const std::string& filePath) {
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

void AnimationSystem::ReadAnimationData(Animation* animation) {

	if (!animation) {
		Logger::Log("[kError] AS :: ReadAnimationData: animation pointer is null.");
		return;
	}

	animationData_ = animation;
	allMaxTime_ = animation->duration;
	allStartTime_ = animation->startTime;
	ResetTimer();

}

void AnimationSystem::TakeControlObject(Object* object) {

	if (!object) {
		Logger::Log("[kError] AS :: No such object.");
		return;
	}
	controlledObject_ = object;
	instanceObject_->CopyObject(controlledObject_);
}

void AnimationSystem::Update() {

	UpdateInstanceObject();

	ControlleObject();

}

void AnimationSystem::ControlleObject() {

	auto& cObjectTransform = controlledObject_->mainPosition.transform;
	auto& iObjectTransform = instanceObject_->mainPosition.transform;

	cObjectTransform.CopyAniTranFrom(iObjectTransform);

	for (int i = 0; i < controlledObject_->objectParts_.size(); i++) {
		controlledObject_->objectParts_[i].transform.CopyAniTranFrom(instanceObject_->objectParts_[i].transform);
	}
}

void AnimationSystem::SetTime(float time_) {

	/// 時間を制限内に収める
	nowTime_ = time_;
	if (nowTime_ < allStartTime_)nowTime_ = allStartTime_;
	if (nowTime_ > allMaxTime_)nowTime_ = allMaxTime_;

}

void AnimationSystem::UpdateInstanceObject() {

	for (size_t nodeNum = 0; nodeNum < animationData_->nodeList.size(); nodeNum++) {

		auto& nowNodeList = animationData_->nodeList[nodeNum];

		auto& part = instanceObject_->objectParts_[nodeNum].transform;

		part.aniScale = MakeTimeValue(nowNodeList.scaleList, nowTime_);
		part.aniRotate = MakeTimeValue(nowNodeList.rotateList, nowTime_);
		part.aniTranslate = MakeTimeValue(nowNodeList.translationList, nowTime_);

		part.isAnimated = true;


		instanceObject_->objectParts_[nodeNum].transform.isAnimated = true;
	}
}

Animation AnimationSystem::LoadAnimationDataFromGltf(const std::string& filePath) {

	std::ifstream input(filePath);
	if (!input.is_open()) {
		throw std::runtime_error("Failed to open file: " + filePath);
	}

	/// ファイルを読み込む
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);

	/// アニメーションデータがない
	if (scene->mNumAnimations != 0) {
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
		for (int keyIndex = 0; keyIndex < nodeAnim->mNumScalingKeys; keyIndex++) {
			KeyFrameVector3 keyFrame;
			keyFrame.time_ = float(nodeAnim->mScalingKeys[keyIndex].mTime);
			keyFrame.value_ = { nodeAnim->mScalingKeys[keyIndex].mValue.x,
								nodeAnim->mScalingKeys[keyIndex].mValue.y,
								nodeAnim->mScalingKeys[keyIndex].mValue.z, };
			nodeData.scaleList.push_back(keyFrame);
		}
		/// 回転のKeyFrameを保存
		for (int keyIndex = 0; keyIndex < nodeAnim->mNumRotationKeys; keyIndex++) {
			KeyFrameQuaternion keyFrame;
			keyFrame.time_ = float(nodeAnim->mRotationKeys[keyIndex].mTime);
			Quaternion quatValue = { nodeAnim->mRotationKeys[keyIndex].mValue.x,
									 nodeAnim->mRotationKeys[keyIndex].mValue.y,
									 nodeAnim->mRotationKeys[keyIndex].mValue.z,
									 nodeAnim->mRotationKeys[keyIndex].mValue.w, };
			keyFrame.value_ = quatValue;
			nodeData.rotateList.push_back(keyFrame);
		};
		/// 平行移動のKeyFrameを保存
		for (int keyIndex = 0; keyIndex < nodeAnim->mNumPositionKeys; keyIndex++) {
			KeyFrameVector3 keyFrame;
			keyFrame.time_ = float(nodeAnim->mPositionKeys[keyIndex].mTime);
			keyFrame.value_ = { nodeAnim->mPositionKeys[keyIndex].mValue.x,
								nodeAnim->mPositionKeys[keyIndex].mValue.y,
								nodeAnim->mPositionKeys[keyIndex].mValue.z, };
			nodeData.translationList.push_back(keyFrame);
		}
		result.nodeList.push_back(nodeData);
	}

	return result;
}

Animation AnimationSystem::LoadAnimationDataFromJson(const std::string& filePath) {
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

void AnimationSystem::ResetTimer() {
}

float AnimationSystem::ChangeEasing(AnimationType type, float t, float rate) {

	switch (type) {
	case AnimationType::LINEARITY:
		return linearity(0, 1, t);
	case AnimationType::EASY_IN:
		return easyIn(0, 1, t, rate);
	case AnimationType::EASY_OUT:
		return easyOut(0, 1, t, rate);
	case AnimationType::EASY_IN_OUT:
		return easyInOut(0, 1, t, rate);
	case AnimationType::EASY_IN_BACK:
		return easyInBack(0, 1, t, rate);
	case AnimationType::EASY_OUT_BACK:
		return easyOutBack(0, 1, t, rate);
	}
	return 0;
}