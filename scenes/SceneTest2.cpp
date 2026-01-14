#include "SceneTest2.h"

SceneTest2::SceneTest2(kEngine* system) {
	/// =========== システム初期化 ============///
	system_ = system;
	debugCamera_ = new DebugCamera(system);
	camera_ = new Camera;
	camera_->Move(Vector3(0.0f, 0.5f, -10.0f));

	/// =========== リソースロード ============///
	skydomeModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");
	sphereModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/plane/plane.obj");

	boxTextureHandle_ = system_->LoadTexture("resources/texture/testBox.png");
	tryTextureHandle_ = system_->LoadTexture("resources/texture/Tryer.png");

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

	sprite_ = new SimpleSprite;
	sprite_->IntObject(system_);
	sprite_->CreateDefaultData();
	sprite_->objectParts_[0].materialConfig->textureHandle = tryTextureHandle_;

}

SceneTest2::~SceneTest2() {
	delete camera_;
	delete debugCamera_;

	delete skydome_;
	delete sprite_;
	for (auto& ptr : plane_) {
		delete ptr;
	}
}

void SceneTest2::Update() {
	/// カメラ処理
	CameraPart();

	/// Skydome更新
	skydome_->Update(usingCamera_);

	for (auto& ptr : plane_) {
		ptr->Update(usingCamera_);
	}

	sprite_->Update(usingCamera_);

	if (system_->GetTriggerOn(DIK_0)) {
		if (useDebugCamera)useDebugCamera = false;
		else useDebugCamera = true;
	}
}


void SceneTest2::Draw() {

	/// 実体処理
	//system_->DrawModel(&skydome_->objectParts[0].transformationMatrix, &(skydome_->objectParts[0].materialConfig.get()), skydomeModelHandle_);
	skydome_->Draw();
	for (auto& ptr : plane_) {
		ptr->Draw();
	}
	sprite_->Draw();

#ifdef USE_IMGUI
	/// imgui処理
	ImguiPart();
#endif
}

void SceneTest2::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
	} else {
		usingCamera_ = camera_;
	}
	usingCamera_->Update();
}

#ifdef USE_IMGUI
void SceneTest2::ImguiPart() {
	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse", &useDebugCamera);
	ImGui::End();

	{
		float fps = system_->GetFPS();
		float fps1s = system_->GetFPSPerSecond();
		float deltaTime = system_->GetDeltaTime();
		ImGui::Begin("FPS");
		ImGui::InputFloat("FPS", &fps);
		ImGui::InputFloat("FPS_1s", &fps1s);
		ImGui::InputFloat("deltaTime", &deltaTime);
		ImGui::End();
	}

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

	{
		ImGui::Begin("SpritePos");
		ImGui::SliderFloat3("Pos", &sprite_->mainPosition.transform.translate.x, 0.0f, 1280.0f);
		ImGui::SliderFloat3("Rotate", &sprite_->mainPosition.transform.rotate.x, 0.0f, 6.5f);
		ImGui::SliderFloat3("Scale", &sprite_->mainPosition.transform.scale.x, 0.0f, 5.0f);

		ImGui::SliderFloat3("AnchorPoint", &sprite_->mainPosition.worldAnchorPoint.x, -500.0f, 500.0f);

		ImGui::Text("MaterialConfig");
		ImGui::SliderFloat3("uvTranslate", &sprite_->objectParts_[0].materialConfig->uvTranslate.x, -5.0f, 5.0f);
		ImGui::SliderFloat3("uvScale", &sprite_->objectParts_[0].materialConfig->uvScale.x, -5.0f, 5.0f);
		ImGui::SliderFloat3("uvRotate", &sprite_->objectParts_[0].materialConfig->uvRotate.x, -6.5f, 6.5f);
		ImGui::ColorEdit4("Color", &sprite_->objectParts_[0].materialConfig->textureColor.x);
		ImGui::End();
	}
}
#endif