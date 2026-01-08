#include "BackPoint.h"

BackPoint::BackPoint(kEngine* system, const Vector3& position) {
	IntObject(system);

	/// モデルハンドル
	MH_Model_ = system_->SetModelObj("resources/object/backpoint/backpoint.obj");
	
	CreateModelData(MH_Model_);
	basePosition_ = position;
	mainPosition.transform.translate = position;
	mainPosition.transform.scale = { 0.5f, 0.5f, 0.5f };


	animationTimer_.Init0(kBackPointAnimationTime, system_->GetTimeManager());

}

BackPoint::~BackPoint() {
}

void BackPoint::Update(Camera* camera) {
 	
	mainPosition.transform.rotate.y += kBackPointTurnTime * system_->GetDeltaTime();

	if (isTimeUp_) {
		animationTimer_.ToMix();
		if (animationTimer_.parameter_ == animationTimer_.maxTime_)isTimeUp_ = false;
	} else {
		animationTimer_.ToZero();
		if (animationTimer_.parameter_ == 0.0f)isTimeUp_ = true;
	}
	float t = easyInOut(0.0f, 1.0f, animationTimer_.parameter_, animationTimer_.maxTime_, 1.2f);

	objectParts_[0].transform.translate.y = -0.5f + 0.5f * t;
	objectParts_[0].transform.scale.y = 0.5f + 0.5f * t;
	objectParts_[0].transform.scale.z = 1.0f + 0.5f * (1.0f - t);
	objectParts_[0].transform.scale.x = 1.0f + 0.5f * (1.0f - t);
	objectParts_[0].materialConfig->textureColor.w = 0.5f + 0.5f * t;

	Object::Update(camera);
}

AABB BackPoint::GetAABB() {
	
	const Vector3& s = mainPosition.transform.scale;
	float halfWidth = 0.5f * std::abs(s.x);
	float halfHeight = 0.5f * std::abs(s.y);
	float halfDepth = 0.5f * std::abs(s.z);
	hitBox_.min = mainPosition.transform.translate - Vector3(halfWidth, halfHeight, halfDepth);
	hitBox_.max = mainPosition.transform.translate + Vector3(halfWidth, halfHeight, halfDepth);
	return hitBox_;
}
