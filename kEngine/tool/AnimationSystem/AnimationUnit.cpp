#include "AnimationUnit.h"
#include "kEngine.h"
#include "externals/nlohmann/json.hpp"
#include <Camera/Camera.h>

AnimationUnit::AnimationUnit(kEngine* system) {
	system_ = system;

	nowTime_ = 0.0f;

	instanceObject_ = std::make_unique<Object>();
	instanceObject_->IntObject(system_);
}

AnimationUnit::~AnimationUnit() {
}

AnimationUnit* AnimationUnit::ReadAnimationData(std::shared_ptr<Animation> animation) {

	if (!animation) {
		Logger::Log("[kError] AS :: ReadAnimationData: animation pointer is null.");
		return nullptr;
	}

	animationData_ = animation;
	allMaxTime_ = animation->duration;
	allStartTime_ = animation->startTime;

	return this;
}

void AnimationUnit::TakeControlObject(Object* object) {

	if (!object) {
		Logger::Log("[kError] AS :: No such object.");
		return;
	}

	controlledObject_ = object;
	instanceObject_->CopyObject(controlledObject_);

	BindingAnimationNodeToObjectPart();

}

void AnimationUnit::Update() {

	UpdateInstanceObject();

	ControlleObject();

}

void AnimationUnit::ControlleObject() {

	if (!isObjectChange_)return;

	auto& cObjectTransform = controlledObject_->mainPosition.transform;
	auto& iObjectTransform = instanceObject_->mainPosition.transform;

	cObjectTransform.CopyAniTranFrom(iObjectTransform);

	int partCount = std::min(
		(int)controlledObject_->objectParts_.size(),
		(int)instanceObject_->objectParts_.size()
	);

	for (int i = 0; i < partCount; i++) {
		controlledObject_->objectParts_[i].transform.CopyAniTranFrom(instanceObject_->objectParts_[i].transform);
	}
}

void AnimationUnit::SetTime(float time_) {

	/// 時間を制限内に収める
	nowTime_ = time_;
	if (nowTime_ < allStartTime_)nowTime_ = allStartTime_;
	if (nowTime_ > allMaxTime_)nowTime_ = allMaxTime_;

}

void AnimationUnit::UpdateInstanceObject() {

	auto anim = animationData_.lock();
	/// アニメーションがなければ終わる
	if (!anim) {
		isObjectChange_ = false;
		return;
	}

	for (size_t i = 0; i < anim->nodeList.size(); i++) {

		if (animationBindings.animToObject.empty()) {
			isObjectChange_ = false;
			return;
		}

		int objIndex = animationBindings.animToObject[i];

		// nodeに対応するobject partがない場合はスキップ
		if (objIndex < 0 || objIndex >= instanceObject_->objectParts_.size()) {
			continue;
		}

		auto& node = anim->nodeList[i];
		auto& part = instanceObject_->objectParts_[objIndex].transform;

		part.aniScale = MakeTimeValue(node.scaleList, nowTime_);
		part.aniRotate = MakeTimeValue(node.rotateList, nowTime_);
		part.aniTranslate = MakeTimeValue(node.translationList, nowTime_);

		part.isAnimated = true;
	}
	isObjectChange_ = true;
}


float AnimationUnit::ChangeEasing(AnimationType type, float t, float rate) {

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

void AnimationUnit::BindingAnimationNodeToObjectPart() {

	auto anim = animationData_.lock();
	if (!anim) return;

	animationBindings.animToObject.resize(animationData_.lock()->nodeList.size(), -1);

	for (int i = 0; i < anim->nodeList.size(); i++) {

		const std::string& nodeName = anim->nodeList[i].name;

		auto target = std::find_if(
			controlledObject_->objectParts_.begin(),
			controlledObject_->objectParts_.end(),
			[&](const ObjectPart& object) {
				return nodeName == object.name;
			}
		);

		/// partのIndexをとる
		int partIndex = (int)std::distance(controlledObject_->objectParts_.begin(), target);

		/// アニメーションのノードが見つかったら、partIndexとnodeIndexを紐づける
		if (target != controlledObject_->objectParts_.end()) {

			animationBindings.animToObject[i] = partIndex;
		} else {
			Logger::Log("[kWarning] AS_Unit :: No object part found for animation node: " + nodeName);
			animationBindings.animToObject[i] = -1; // No animation
		}
	}
}