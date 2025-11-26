#include "Effect1.h"

Effect1::Effect1(kEngine* system) {
	/// =========== システム初期化 ============///
	system_ = system;
	debugCamera_ = new DebugCamera(system);
	camera_ = new Camera;
	camera_->Move(Vector3(0.0f, 0.5f, -10.0f));

	/// =========== リソースロード ============///
	skydomeModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");
	sphereModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/plane/plane.obj");

	boxTextureHandle_ = system_->LoadTextrue("resources/texture/testBox.png");

	skydome_ = new Object;
	skydome_->IntObject(system_);
	skydome_->CreateModelData(skydomeModelHandle_);
	skydome_->objectParts_[0].materialConfig->enableLighting = false;

	int i = 0;
	for (auto& ptr : plane_) {
		ptr = new Object;
		ptr->IntObject(system_);
		ptr->CreateModelData(sphereModelHandle_);
		ptr->mainPosition.transform.translate = Vector3(0.2f * i, 0.2f * i, 0.2f * i);
		i++;
	}


}

Effect1::~Effect1() {
	delete camera_;
	delete debugCamera_;

	delete skydome_;
	for (auto& ptr : plane_) {
		delete ptr;
	}
}

void Effect1::Update() {
	/// カメラ処理
	CameraPart();

	/// Skydome更新
	skydome_->Update(usingCamera_);

	for (auto& ptr : plane_) {
		ptr->Update(usingCamera_);
	}


	if (system_->GetTriggerOn(DIK_0)) {
		if (useDebugCamera)useDebugCamera = false;
		else useDebugCamera = true;
	}
}


void Effect1::Draw() {

	/// 実体処理
	//system_->DrawModel(&skydome_->objectParts[0].transformationMatrix, &(skydome_->objectParts[0].materialConfig.get()), skydomeModelHandle_);
	skydome_->Draw();
	for (auto& ptr : plane_) {
		ptr->Draw();
	}

#ifdef USE_IMGUI
	/// imgui処理
	ImguiPart();
#endif
}

void Effect1::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
	} else {
		usingCamera_ = camera_;
	}
	usingCamera_->Update();
}

#ifdef USE_IMGUI
void Effect1::ImguiPart() {
	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse", &useDebugCamera);
	ImGui::End();


	{
		ImGui::Begin("SpherePos");
		int i = 0;
		for (auto& ptr : plane_) {
			std::string index = std::to_string(i);
			std::string name1 = "SpherePos";
			std::string name2 = "SpherePos";
			ImGui::SliderFloat4((name1 + index).c_str(), &ptr->objectParts_[0].materialConfig->textureColor.x, 0.0f, 1.0f);
			i++;
		}
		ImGui::End();
	}
}
#endif