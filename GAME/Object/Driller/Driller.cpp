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
}

void Driller::Update(Camera* camera) {

	/// 状態による処理
	if ((drillerState_ & (int)DrillerState::kTitleMovement) == (int)DrillerState::kTitleMovement) {
		TitleMovement();
	} else {
		sharkObject_->ReleaseObject();
		isShake_ = false;
	}

	/// 振動処理
	if (isShake_) {
		if (sharkObject_->GetTargetFollow() != this) {
			sharkObject_->TakeAppartObject(this);
		}
		sharkObject_->Update(nullptr);
	}
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

void Driller::OpenDoor() {}

void Driller::CloseDoor() {}

void Driller::RunDrill() {

	/// ドリル回転
	this->objectParts_[3].transform.rotate.y += 0.05f;

}

