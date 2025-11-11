#include "SceneTest.h"

SceneTest::SceneTest(kEngine* system) {
	system_ = system;
	debugCamera_ = new DebugCamera(system);
	camera_ = new Camera;

	player_->CreateDefaultData();
}

SceneTest::~SceneTest() {
	delete camera_;
	delete debugCamera_;
}

void SceneTest::Update() {
	/// カメラ処理
	CameraPart();

	/// player更新
	player_->Update(usingCamera_);
}


void SceneTest::Draw() {

	/// 実体処理
	system_->DrawCube(&player_->transformationMatrix, player_->materialConfig[0]);
	
	/// imgui処理
	ImguiPart();
}

void SceneTest::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
	} else {
		usingCamera_ = camera_;
	}
	usingCamera_->Update();
}

void SceneTest::ImguiPart() {
	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse",&useDebugCamera);
	ImGui::End();
}
