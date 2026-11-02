#include "sign.h"
#include "kEngine.h"

Sign::Sign(kEngine* system, const Vector3& position) {
	IntObject(system);

	/// モデルハンドル
	MH_Model_ = system_->SetModelObj("GAME/resources/object/sign/sign.gltf");

	CreateModelData(MH_Model_);
	cellPosition_ = position;
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
		if (t < kSquash) {
			float instantT = t / kSquash;
			mainPosition.transform.scale.x = linearity(kBase, ChangeSmallScale_.x, instantT);
			mainPosition.transform.scale.y = linearity(kBase, ChangeSmallScale_.y, instantT);
		} else {
			float instantT = (t - kSquash) / (1.0f - kSquash);
			mainPosition.transform.scale.x = linearity(ChangeSmallScale_.x, kBase, instantT);
			mainPosition.transform.scale.y = linearity(ChangeSmallScale_.y, kBase, instantT);
		}
	} else {
		/// 終わったらぴったり戻す（最後のフレームで少しずれたまま止まるのを防ぐ）
		mainPosition.transform.scale.x = kBase;
		mainPosition.transform.scale.y = kBase;
	}
}

AABB Sign::GetAABB() {
	/// 判定はマスの中心＋固定サイズ（縮むアニメーションの影響を受けない）
	hitBox_.min = cellPosition_ - kSignInteractHalfSize;
	hitBox_.max = cellPosition_ + kSignInteractHalfSize;
	return hitBox_;
}

bool Sign::IsPlayerNear(const Vector3& playerPos) const {
	return std::abs(playerPos.x - cellPosition_.x) <= kSignInteractRange &&
		std::abs(playerPos.y - cellPosition_.y) <= kSignInteractRange;
}
