#include "SceneTest.h"

SceneTest::SceneTest(kEngine* system){
	/// =========== システム初期化 ============///
	system_ = system;
	debugCamera_ = new DebugCamera(system);
	camera_ = new Camera;
	camera_->Move(Vector3(0.0f, 0.5f, -10.0f));

	/// =========== リソースロード ============///
	skydomeModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");

	boxTextureHandle_ = system_->LoadTextrue("resources/texture/testBox.png");
	tryTextureHandle_ = system_->LoadTextrue("resources/texture/Tryer.png");

	//skydome_->CreateDefaultData();
	skydome_ = new Object;
	skydome_->IntObject(system_);
	skydome_->CreateModelData(skydomeModelHandle_);
	skydome_->objectParts_[0].materialConfig->enableLighting = false;

	player_ = new Player(system,Vector3(0,0.5f,0));
	player_->CreateDefaultData();
	player_->modelHandle_ = config::default_Cube_MeshBufferHandle_;
	player_->objectParts_[0].materialConfig->useModelTexture = false;
	player_->objectParts_[0].materialConfig->textureHandle = boxTextureHandle_;

	sprite_ = new SimpleSprite;
	sprite_->IntObject(system_);
	sprite_->CreateDefaultData();
	sprite_->objectParts_[0].materialConfig->textureHandle = tryTextureHandle_;

	sprite2_ = new SimpleSprite;
	sprite2_->IntObject(system_);
	sprite2_->CreateDefaultData();
	sprite2_->objectParts_[0].materialConfig->textureHandle = boxTextureHandle_;

	sprite3_ = new SimpleSprite;
	sprite3_->IntObject(system_);
	sprite3_->CreateDefaultData();
	sprite3_->objectParts_[0].materialConfig->textureHandle = boxTextureHandle_;
	sprite3_->mainPosition.transform.translate = Vector3(200.0f, 200.0f, 0.0f);

	sprite4_ = new SimpleSprite;
	sprite4_->IntObject(system_);
	sprite4_->CreateDefaultData();
	sprite4_->objectParts_[0].materialConfig->textureHandle = boxTextureHandle_;
	sprite4_->mainPosition.transform.translate = Vector3(400.0f, 400.0f, 0.0f);

}

SceneTest::~SceneTest() {
	delete camera_;
	delete debugCamera_;

	delete player_;
	delete skydome_;
	delete sprite_;
	delete sprite2_;
	delete sprite3_;
	delete sprite4_;
}

void SceneTest::Update() {
	/// カメラ処理
	CameraPart();

	/// Skydome更新
	skydome_->Update(usingCamera_);

	/// player更新
	player_->Update(usingCamera_);

	sprite_->Update(usingCamera_);
	sprite2_->Update(usingCamera_);
	sprite3_->Update(usingCamera_);
	sprite4_->Update(usingCamera_);

	if (system_->GetTriggerOn(DIK_0)) {
		if (useDebugCamera)useDebugCamera = false;
		else useDebugCamera = true;
	}
}


void SceneTest::Draw() {

	/// 実体処理
	//system_->DrawModel(&skydome_->objectParts[0].transformationMatrix, &(skydome_->objectParts[0].materialConfig.get()), skydomeModelHandle_);
	skydome_->Draw();
	player_->Draw();
	sprite_->Draw();
	//sprite2_->Draw();
	//sprite3_->Draw();
	//sprite4_->Draw();
	//system_->Draw3D(skydome_);
	//system_->Draw3D(player_);
	//system_->Draw3D(model_);
	
#ifdef USE_IMGUI
	/// imgui処理
	ImguiPart();
#endif
}

void SceneTest::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
	} else {
		usingCamera_ = camera_;
	}
	usingCamera_->Update();
}

#ifdef USE_IMGUI
void SceneTest::ImguiPart() {
	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse",&useDebugCamera);
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
		ImGui::Begin("PlayerPos");
		ImGui::SliderFloat3("Pos", &player_->mainPosition.transform.translate.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Rotate", &player_->mainPosition.transform.rotate.x, -1.0f, 1.0f);
		ImGui::End();
	}

	{
		ImGui::Begin("SpritePos");
		ImGui::SliderFloat3("Pos",		&sprite_->mainPosition.transform.translate.x, 0.0f, 1280.0f);
		ImGui::SliderFloat3("Rotate",	&sprite_->mainPosition.transform.rotate.x, 0.0f, 6.5f);
		ImGui::SliderFloat3("Scale",	&sprite_->mainPosition.transform.scale.x, 0.0f, 5.0f);

		ImGui::SliderFloat3("AnchorPoint",	&sprite_->mainPosition.anchorPoint.x, -500.0f, 500.0f);

		ImGui::Text("MaterialConfig");
		ImGui::SliderFloat3("uvTranslate", & sprite_->objectParts_[0].materialConfig->uvTranslate.x, -5.0f, 5.0f);
		ImGui::SliderFloat3("uvScale", & sprite_->objectParts_[0].materialConfig->uvScale.x, -5.0f, 5.0f);
		ImGui::SliderFloat3("uvRotate", & sprite_->objectParts_[0].materialConfig->uvRotate.x, -6.5f, 6.5f);
		ImGui::ColorEdit4("Color", & sprite_->objectParts_[0].materialConfig->textureColor.x);
		ImGui::End();
	}
}
#endif