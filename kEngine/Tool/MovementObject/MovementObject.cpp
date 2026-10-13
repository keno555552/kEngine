#include "MovementObject.h"

MovementObject::MovementObject(kEngine* system) {

	/// Objectの初期化
	this->IntObject(system);
	this->CreateDefaultData();

	/// 乱数生成器の初期化
	randomMaker_ = std::make_unique<RandomMaker>();

	/// シェーク関連の初期化
	shakingTime_.InitM(0.0f, system_->GetTimeManager());
}

void MovementObject::Update(Camera* camera) {
	ShakeUpdate();
	Object::Update(nullptr);
}

void MovementObject::SetAndResetShakeTime(float time) {
	shakingTime_.InitM(time, system_->GetTimeManager());
}

void MovementObject::SetShakeTime(float time) {
	if (time < 0.0f) return;
	shakingTime_.maxTime_ = time;
}

void MovementObject::TakeAppartObject(Object* object) {

	/// 元の目標と同じ場合は、何もしない
	if (this->followObject_ == &object->mainPosition) return;
	if (object->followObject_ == &this->mainPosition) return;

	/// 目標のfollowと目標の間に自分を挟むにより、目標のフォローを変わらないように見え、このオブションの効果も得られる
	this->followObject_ = object->followObject_;
	object->followObject_ = &(this->mainPosition);

	/// Dismissのときに、目標のfollowをnullにするために、目標のfollowを保持する
	targetFollow_ = object;

}

void MovementObject::ReleaseObject() {
	this->followObject_ = nullptr;
	targetFollow_ = nullptr;
}

/// ==================================== ムーブメント関連の関数 ==================================== ///

void MovementObject::ShakeUpdate() {

	/// タイマーが0でない場合は、タイマーを使う
	if (shakingTime_.maxTime_ != 0.0f) {
		bool isTime = false;
		if (shakingTime_.parameter_ <= 0.0f) {
			isTime = shakingTime_.ToZeroMix();
		}
		/// シェークの時間が0でない場合は、シェークを行わない
		if (!isTime)return;
	}

	if ((state_ & (int)MovementObjectState::kShake) == (int)MovementObjectState::kShake) {
		/// シェーク

		Vector3 shakePos{};
		shakePos.x = randomMaker_->randomFloat(-shakingScale_, shakingScale_);
		shakePos.y = randomMaker_->randomFloat(-shakingScale_, shakingScale_);
		shakePos.z = randomMaker_->randomFloat(-shakingScale_, shakingScale_);

		this->mainPosition.transform.translate = shakePos;
	} else {
		followObject_ = nullptr;
		this->mainPosition.transform.translate = { 0.0f, 0.0f, 0.0f };
	}

}
