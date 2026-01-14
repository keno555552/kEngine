#include "SceneTest.h"

SceneTest::SceneTest(kEngine* system) {
	/// =========== システム初期化 ============///
	system_ = system;
	debugCamera_ = new DebugCamera(system);
	camera_ = new Camera;
	camera_->Move(Vector3(0.0f, 0.5f, -10.0f));
	usingCamera_ = camera_;
	system_->SetCamera(usingCamera_);

	/// =========== リソースロード ============///
	skydomeModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");
	playerModelHandle_ = system_->SetModelObj("resources/object/ball/ball.obj");

	boxTextureHandle_ = system_->LoadTextrue("resources/texture/testBox.png");
	tryTextureHandle_ = system_->LoadTextrue("resources/texture/Tryer.png");
	uvTextureHandle_ = system_->LoadTextrue("resources/TemplateResource/texture/uvChecker.png");

	//skydome_->CreateDefaultData();
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

	sprite_ = new SimpleSprite;
	sprite_->IntObject(system_);
	sprite_->CreateDefaultData();
	sprite_->objectParts_[0].materialConfig->textureHandle = uvTextureHandle_;

	sprite2_ = new SimpleSprite;
	sprite2_->IntObject(system_);
	sprite2_->CreateDefaultData();
	sprite2_->objectParts_[0].materialConfig->textureHandle = uvTextureHandle_;


	/// マップチップの生成
	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("resources/stage/blocks1.csv");
	mapChipField_->SetBlockWidth(1.0f);
	mapChipField_->SetBlockHeight(1.0f);

	/// マップチップの初期化
	GenerateBlocks();
}

SceneTest::~SceneTest() {
	delete camera_;
	delete debugCamera_;

	delete player_;
	delete skydome_;
	delete sprite_;
	delete sprite2_;
	delete mapChipField_;
}

void SceneTest::Update() {
	/// カメラ処理
	CameraPart();

	/// Skydome更新
	skydome_->Update(usingCamera_);

	/// player更新
	player_->Update(usingCamera_);

	sprite_->Update(usingCamera_);
	//sprite2_->Update(usingCamera_);

	/// ブロック更新
	for (auto& row : blockObjectList_) {
		for (auto& block : row) {
			if (block) {
				block->Update(usingCamera_);
			}
		}
	}

	if (system_->GetTriggerOn(DIK_0)) {
		if (useDebugCamera)useDebugCamera = false;
		else useDebugCamera = true;
	}

	if (system_->GetTriggerOn(DIK_SPACE)) {
		ChangeNextStage(SceneNum::S_Result);
	}
}


void SceneTest::Draw() {

	/// 実体処理
	//system_->DrawModel(&skydome_->objectParts[0].transformationMatrix, &(skydome_->objectParts[0].materialConfig.get()), skydomeModelHandle_);
	skydome_->Draw();
	player_->Draw();
	//sprite_->Draw();
	//sprite2_->Draw();
	//system_->Draw3D(skydome_);
	//system_->Draw3D(player_);
	//system_->Draw3D(model_);

	/// ブロック描画
	for (auto& row : blockObjectList_) {
		for (auto& block : row) {
			if (block) {
				block->Draw();
			}
		}
	}

#ifdef USE_IMGUI
	/// ImGui処理
	ImGuiPart();
#endif
}

void SceneTest::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
	} else {
		Transform cameraTransform = CreateDefaultTransform();
		cameraTransform.translate.x = player_->mainPosition.transform.translate.x;
		cameraTransform.translate.y = player_->mainPosition.transform.translate.y + 0.5f;
		cameraTransform.translate.z = player_->mainPosition.transform.translate.z - 15.0f;
		camera_->SetCamera(cameraTransform);
		usingCamera_ = camera_;
	}
	usingCamera_->Update();
	system_->SetCamera(usingCamera_);
}

#ifdef USE_IMGUI
void SceneTest::ImGuiPart() {
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


void SceneTest::GenerateBlocks() {
	/// ボックス生成
	// 要素数
	uint32_t kNumBlockVertical = mapChipField_->GetNumBlockVirtical();
	uint32_t kNumBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	// 列数を設定(縦方向のブロック数)
	blockObjectList_.resize(kNumBlockVertical);
	for (uint32_t i = 0; i < kNumBlockVertical; i++) {
		// 列数を設定(横方向のブロック数)
		blockObjectList_[i].resize(kNumBlockHorizontal);
	}
	// いざボックス生成
	for (uint32_t i = 0; i < kNumBlockVertical; i++) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; j++) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				blockObjectList_[i][j] = new Object();
				blockObjectList_[i][j]->IntObject(system_);
				blockObjectList_[i][j]->CreateDefaultData();
				blockObjectList_[i][j]->modelHandle_ = config::default_Cube_MeshBufferHandle_;
				blockObjectList_[i][j]->objectParts_[0].materialConfig->useModelTexture = false;
				blockObjectList_[i][j]->objectParts_[0].materialConfig->textureHandle = boxTextureHandle_;
				blockObjectList_[i][j]->mainPosition.transform.translate = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}