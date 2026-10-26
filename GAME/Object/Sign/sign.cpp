#include "sign.h"
#include "kEngine.h"

Sign::Sign(kEngine* system, const Vector3& position) {
	IntObject(system);

	/// モデルハンドル
	MH_Model_ = system_->SetModelObj("GAME/resources/object/sign/sign.gltf");

	CreateModelData(MH_Model_);
	mainPosition.transform.translate = position;
	mainPosition.transform.translate.y -= 0.5f;
	mainPosition.transform.scale = { 0.5f, 0.5f, 0.5f };


	animationTimer_.InitM(kSignOPEDTime, system_->GetTimeManager());

}

Sign::~Sign() {}

void Sign::Update(Camera* camera) {
	camera;

	/// Timerの更新
	if (isInteract_) {
		animationTimer_.Reset0();
		isInteract_ = false;
	}

	/// 前 30% で縮む、残り 70% で戻る
	const float kSquash = 0.3f;
	const float kBase = 0.5f;
	if (!animationTimer_.ToMix()) {
		float t = animationTimer_.linearity();
		if (t >= 0.3f) {
			float instrantT = (t / kSquash);
			mainPosition.transform.scale.x = linearity(kSquash, ChangeSmallScale_.x, instrantT);
			mainPosition.transform.scale.y = linearity(kSquash, ChangeSmallScale_.y, instrantT);
		} else {
			float instrantT = ((t - kSquash) / (1.0f - kSquash));
			mainPosition.transform.scale.x = linearity(ChangeSmallScale_.x, kBase, instrantT);
			mainPosition.transform.scale.y = linearity(ChangeSmallScale_.y, kBase, instrantT);
		}
	}
}

AABB Sign::GetAABB() {

	const Vector3& s = mainPosition.transform.scale;
	float halfWidth = 0.5f * std::abs(s.x);
	float halfHeight = 0.5f * std::abs(s.y);
	float halfDepth = 0.5f * std::abs(s.z);
	hitBox_.min = mainPosition.transform.translate - Vector3(halfWidth, halfHeight, halfDepth);
	hitBox_.max = mainPosition.transform.translate + Vector3(halfWidth, halfHeight, halfDepth);
	return hitBox_;
}
