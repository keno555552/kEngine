#pragma once
#include <vector>
#include "MathsIncluder.h"
#include "Data/Animation/Keyframe.h"

/// 一つのアニメーションのノードデータ
struct AnimationNodeData {

	int animationNodeID{};
	std::string name;

	std::vector <KeyFrameVector3> scaleList{};
	std::vector <KeyFrameQuaternion> rotateList{};
	std::vector <KeyFrameVector3> translationList{};
	AnimationNodeData* parent_ = nullptr;

	Object SimpleObject{};
	void SetSimpleObject(const Object& obj);

	enum class KeyFrameType {
		SCALE,
		ROTATE,
		TRANSLATION,
		NumOfType,
	};
	void AddKeyFrame(int keyFrameType, float time_ = 0);
};
