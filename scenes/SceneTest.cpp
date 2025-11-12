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

	skydome_->CreateDefaultData();

	player_ = new Player(system,Vector3(0,0.5f,0));
	player_->CreateDefaultData();
	player_->materialConfig[0].useOriginalTexture = false;
	player_->materialConfig[0].textureHandle = boxTextureHandle_;
}

SceneTest::~SceneTest() {
	delete camera_;
	delete debugCamera_;
}

void SceneTest::Update() {
	/// カメラ処理
	CameraPart();

	/// Skydome更新
	skydome_->Update(usingCamera_);

	/// player更新
	player_->Update(usingCamera_);

	if (system_->GetTriggerOn(DIK_0)) {
		if (useDebugCamera)useDebugCamera = false;
		else useDebugCamera = true;
	}
}


void SceneTest::Draw() {

	/// 実体処理
	system_->DrawModel(&skydome_->transformationMatrix, skydome_->materialConfig, skydomeModelHandle_);
	system_->DrawCube(&player_->transformationMatrix, player_->materialConfig[0]);
	
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
		ImGui::SliderFloat3("Pos", &player_->transform.translate.x, -1.0f, 1.0f);
		ImGui::End();
	}
}
#endif