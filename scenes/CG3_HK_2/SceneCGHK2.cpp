#include "SceneCGHK2.h"

SceneCGHK2::SceneCGHK2(kEngine* system) {
	/// =========== システム初期化 ============///
	system_ = system;
	debugCamera_ = system_->CreateDebugCamera();
	camera_ = system_->CreateCamera();
	camera_->Move(Vector3(0.0f, 0.5f, -10.0f));
	usingCamera_ = camera_;
	system_->SetCamera(usingCamera_);

	/// =========== リソースロード ============///
	skydomeModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");
	playerModelHandle_ = system_->SetModelObj("resources/object/ball/ball.obj");

	boxTextureHandle_ = system_->LoadTexture("resources/texture/testBox.png");
	tryTextureHandle_ = system_->LoadTexture("resources/texture/Tryer.png");
	uvTextureHandle_ = system_->LoadTexture("resources/TemplateResource/texture/uvChecker.png");

	skydome_ = new Object;
	skydome_->IntObject(system_);
	skydome_->CreateModelData(skydomeModelHandle_);
	skydome_->objectParts_[0].materialConfig->enableLighting = false;

	player_ = new Player(system, Vector3(2.0f, 0.5f, 0));
	player_->CreateModelData(playerModelHandle_);
	player_->mainPosition.transform.scale = Vector3(0.5f, 0.5f, 0.5f);
	for (auto parts : player_->objectParts_) {
		parts.materialConfig->lightModelType = LightModelType::PhongReflection;
	}

	//player_->CreateDefaultData();
	//player_->modelHandle_ = playerModelHandle_;
	debugObject_ = new DebugObject(system_);
	debugObject_->SetFollowObject(&player_->mainPosition);
	debugObject_->SetShowCenterPoint(true);

	sprite_ = new SimpleSprite;
	sprite_->IntObject(system_);
	sprite_->CreateDefaultData();
	sprite_->objectParts_[0].materialConfig->textureHandle = uvTextureHandle_;

	sprite2_ = new SimpleSprite;
	sprite2_->IntObject(system_);
	sprite2_->CreateDefaultData();
	sprite2_->objectParts_[0].materialConfig->textureHandle = uvTextureHandle_;


}

SceneCGHK2::~SceneCGHK2() {
	system_->DestroyCamera(camera_);
	system_->DestroyCamera(debugCamera_);

	delete debugObject_;
	delete player_;
	delete skydome_;
	delete sprite_;
	delete sprite2_;
}

void SceneCGHK2::Update() {

	CameraPart();

	/// Skydome更新
	skydome_->Update(usingCamera_);

	/// player更新
	player_->Update(usingCamera_);

	sprite_->Update(usingCamera_);

	debugObject_->Update(usingCamera_);


	if (system_->GetTriggerOn(DIK_0)) {
		if (useDebugCamera)useDebugCamera = false;
		else useDebugCamera = true;
	}

	if (system_->GetTriggerOn(DIK_SPACE)) {
		ChangeNextStage(SceneNum::S_Result);
	}
}


void SceneCGHK2::Draw() {

	/// 実体処理
	//system_->DrawModel(&skydome_->objectParts[0].transformationMatrix, &(skydome_->objectParts[0].materialConfig.get()), skydomeModelHandle_);
	skydome_->Draw();
	player_->Draw();
	debugObject_->Draw();
	//sprite_->Draw();
	//sprite2_->Draw();
	//system_->Draw3D(skydome_);
	//system_->Draw3D(player_);
	//system_->Draw3D(model_);


#ifdef USE_IMGUI
	/// ImGui処理
	ImGuiPart();
#endif
}

void SceneCGHK2::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
		debugCamera_->MouseControlUpdate();
	} else {
		//Transform cameraTransform = CreateDefaultTransform();
		//cameraTransform.translate.x = player_->mainPosition.transform.translate.x;
		//cameraTransform.translate.y = player_->mainPosition.transform.translate.y + 0.5f;
		//cameraTransform.translate.z = player_->mainPosition.transform.translate.z - 15.0f;
		//camera_->SetCamera(cameraTransform);
		usingCamera_ = camera_;
	}
	//usingCamera_->Update();
	system_->SetCamera(usingCamera_);
}

#ifdef USE_IMGUI
void SceneCGHK2::ImGuiPart() {
	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse", &useDebugCamera);
	ImGui::End();

	{
		float shininess = player_->objectParts_[0].materialConfig->shininess;
		float scale = player_->mainPosition.transform.scale.x;
		ImGui::Begin("PlayerPos");
		ImGui::SliderFloat3("Pos", &player_->mainPosition.transform.translate.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Rotate", &player_->mainPosition.transform.rotate.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Scale", &player_->mainPosition.transform.scale.x, -1.0f, 1.0f);
		ImGui::SliderFloat("ScaleOnce", &scale, -1.0f, 1.0f);
		ImGui::SliderFloat("shininess", &shininess, 0.0f, 256.0f);
		ImGui::End();

		//player_->mainPosition.transform.scale.x = scale;
		//player_->mainPosition.transform.scale.y = scale;
		//player_->mainPosition.transform.scale.z = scale;

		for (auto& parts : player_->objectParts_) {
			parts.materialConfig->shininess = shininess;
		}
	}

}
#endif