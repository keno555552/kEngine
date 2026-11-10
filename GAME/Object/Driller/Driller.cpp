#include "Driller.h"

Driller::Driller(kEngine* system, const Vector3& position) {

	Object::IntObject(system);
	Object::CreateDefaultData();

	shakeObject_ = std::make_unique<Object>();
	shakeObject_->IntObject(system);
	shakeObject_->CreateDefaultData();

	mainPosition.transform.translate = position;
	objectParts_[0].materialConfig->lightModelType = LightModelType::HalfLambert;

	mainPosition.transform.rotate.y = 0.0f;

	SH_OpenCloseDoor_ = 0;
	SH_RunningDrill_ = 0;

	doorAnimationTimer_.Init0(kDrillerDoorTime, system_->GetTimeManager());

	drillOnOffTimer_.InitM(Driller_OnOffTime, system_->GetTimeManager());

	sharkObject_ = std::make_unique<MovementObject>(system_);
	sharkObject_->SetShake(true, 0.005f);

	/// bugCheck
	checkTimer_.Init0(1.0f, system_->GetTimeManager());

	randomMaker_ = std::make_unique<RandomMaker>();

}

void Driller::TitleMovement() {
	isShake_ = true;
	RunDrill();
	MoveTrack(0.01f);
}

void Driller::Update(Camera* camera) {
	camera;
	totelDeltaTime_ = system_->GetDeltaTime();

	/// 状態による処理
	if ((drillerState_ & (int)DrillerState::kTitleMovement) == (int)DrillerState::kTitleMovement) {
		TitleMovement();
	} else {
		isShake_ = isDrilling_;          // ドリルが回っている間は揺れる
		if (isDrilling_) RunDrill();
		MoveUpdate();
	}
	DoorUpdate();   // Title でも使える

	/// 振動処理
	if (isShake_) {
		if (sharkObject_->GetTargetFollow() != this) {
			sharkObject_->TakeAppartObject(this);
		}
		sharkObject_->Update(nullptr);
	} else {
		sharkObject_->ReleaseObject();
	}
}

void Driller::Draw() {
	if (!isVisible_) return;
	Object::Draw();
}

#ifdef USE_IMGUI
void Driller::ImGuiPart() {

	ImGui::Begin("ObjectParts");
	ImGui::SliderFloat3("Object Position", &mainPosition.transform.translate.x, -10.0f, 10.0f);
	int id = 0;
	for (auto& part : this->objectParts_) {

		ImGui::Separator();
		ImGui::Text("PartName: %s", part.name.c_str());

		std::string idC = "[" + std::to_string(id) + "]";

		std::string transformLable = idC + "Rotate";
		ImGui::SliderFloat3(transformLable.c_str(), &part.transform.rotate.x, -10.0f, 10.0f);
		ImGui::NewLine();

		std::string UVTransformLable = idC + "UVtransform";
		ImGui::SliderFloat3(UVTransformLable.c_str(), &part.materialConfig->uvTranslate.x, -10.0f, 10.0f);
		ImGui::NewLine();

		id++;
	}
	ImGui::Separator();
	ImGui::End();

}
#endif

/// ============================== 落下 ============================== ///
void Driller::DrillDown() {
	isFalling_ = true;
	isDrilling_ = true;
	onGround_ = false;
	velocity_.y = 0.0f;
}

void Driller::StopFall() {
	isFalling_ = false;
	isDrilling_ = false;
	velocity_.y = 0.0f;
	onGround_ = true;
}

void Driller::MoveUpdate() {
	if (!isFalling_) return;

	velocity_.y -= kDrillerGravity * totelDeltaTime_;
	velocity_.y = std::max(velocity_.y, -kDrillerLimitFallSpeed);

	/// 今は自分で動く（衝突を入れたら Player と同じく CollisionManager の結果で動かす）
	mainPosition.transform.translate += velocity_ * totelDeltaTime_;

	/// 【仮】衝突の代わり：生成点の地面で止まる
	const float landY = spawnPoint_.y + kDrillerLandOffsetY;
	if (hasSpawnPoint_ && mainPosition.transform.translate.y <= landY) {
		mainPosition.transform.translate.y = landY;
		StopFall();
	}
}

/// ============================== ドア ============================== ///
void Driller::OpenDoor() {
	if (doorState_ == DoorState::kOpen || doorState_ == DoorState::kOpening) return;
	doorState_ = DoorState::kOpening;
}

void Driller::CloseDoor() {
	if (doorState_ == DoorState::kClosed || doorState_ == DoorState::kClosing) return;
	doorState_ = DoorState::kClosing;
}

void Driller::Call() {
	if (!hasSpawnPoint_ || isCalled_) return;
	isCalled_ = true;
	mainPosition.transform.translate = spawnPoint_ + Vector3{ 0.0f, kDrillerCallHeight, 0.0f };
	velocity_ = {};
	Show();
	DrillDown();
}

void Driller::DoorUpdate() {
	switch (doorState_) {
	case DoorState::kOpening:
		if (doorAnimationTimer_.ToMix()) doorState_ = DoorState::kOpen;
		break;
	case DoorState::kClosing:
		if (doorAnimationTimer_.ToZero()) doorState_ = DoorState::kClosed;
		break;
	default:
		return;   // 動いていない時は触らない
	}
	SetDoorAngle(doorAnimationTimer_.linearity());
}

void Driller::SetDoorAngle(float t) {
	const float angle = kDrillerDoorOpenAngle * t;
	if (ObjectPart* doorL = FindPartByName("doorL.001")) doorL->transform.rotate.y = +angle;
	if (ObjectPart* doorR = FindPartByName("doorR.001")) doorR->transform.rotate.y = -angle;
}

/// ============================ Drill運行 ============================ ///

void Driller::RunDrill() {

	/// ドリルのパーツを取得
	ObjectPart* drillPart = nullptr;
	if (!this->objectParts_.empty()) drillPart = this->FindPartByName("drill.001");

	/// ドリル回転
	if (drillPart) {
		drillPart->transform.rotate.y += kDrillerDrillSpinSpeed * system_->GetDeltaTime();
	}

}

/// =========================== トラック運行 ============================ ///

void Driller::MoveTrack(float UVTransform) {

	/// トラックのパーツを取得
	ObjectPart* trackPart = nullptr;

	int trackNo = 1;
	int trackNo_No = 1;
	for(auto& part: this->objectParts_){

		std::string name = "track" + std::to_string(trackNo) + '_' + std::to_string(trackNo_No) + ".001";
		if (!this->objectParts_.empty()) trackPart = this->FindPartByName(name);

		/// ドリル回転
		if (trackPart) {
			trackPart->materialConfig->uvTranslate.y += UVTransform;

			trackNo_No = (trackNo_No > 1) ? 1 : 2;
			if(trackNo_No == 1)trackNo++;
		}
		if (trackNo > 5)return;
	}


}

