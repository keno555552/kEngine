#include "MovementObject.h"

MovementObject::MovementObject(kEngine* system) {

	/// Objectの初期化
	this->IntObject(system);
	this->CreateDefaultData();

	/// 乱数生成器の初期化
	randomMaker_ = std::make_unique<RandomMaker>();
}

void MovementObject::Update(Camera* camera) {

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
	Object::Update(nullptr);
}

void MovementObject::TakeAppartObject(Object* object) {

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
