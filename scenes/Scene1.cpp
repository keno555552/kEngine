#include "Scene1.h"

Scene1::Scene1(kEngine* system) {
	/// =========== システム初期化 ============///
	system_ = system;
	debugCamera_ = new DebugCamera(system);
	camera_ = new Camera;
	camera_->Move(Vector3(0.0f, 0.5f, -10.0f));

	/// =========== リソースロード ============///
	std::string basePath = "resources/";
	std::string templatePath = "TemplateResource/";
	std::string objectPath = "object/";
	std::string texturePath = "texture/";
	std::string blockPath = "block/";

	std::string soundPath = "sound/";
	std::string BGMPath = "BGM/";
	std::string SEPath = "SE/";

	MH_skydome_ = system_->SetModelObj((basePath + templatePath + objectPath + "skydome/skydome.obj").c_str());
	MH_player_ = system_->SetModelObj((basePath + objectPath + "player/player.obj").c_str());
	MH_targeter_ = system_->SetModelObj((basePath + objectPath + "targeter/targeter.obj").c_str());
	MH_underGround_ = system_->SetModelObj((basePath + objectPath + "underground_BG/underground_BG.obj").c_str());
	MH_bullet_ = system_->SetModelObj((basePath + objectPath + "bullet/bullet.obj").c_str());
	MH_object_ = system_->SetModelObj((basePath + objectPath + "pickaxe/pickaxe.obj").c_str());
	MH_enemy_ = system_->SetModelObj((basePath + objectPath + "enemy/enemy.obj").c_str());
	MH_backPoint_ = system_->SetModelObj((basePath + objectPath + "backSign/backSign.obj").c_str());

	TH_box_ = system_->LoadTexture((basePath + texturePath + "testBox.png").c_str());
	TH_try_ = system_->LoadTexture((basePath + texturePath + "Tryer.png").c_str());
	TH_uv_ = system_->LoadTexture((basePath + templatePath + texturePath + "uvChecker.png").c_str());

	TH_dirt_ = system_->LoadTexture((basePath + texturePath + blockPath + "dirt.png").c_str());
	TH_rock_ = system_->LoadTexture((basePath + texturePath + blockPath + "stone.png").c_str());

	TH_buleSkySkydome_ = system_->LoadTexture((basePath + texturePath + "sky/bluesky.png").c_str());

	TH_hpUI_ = system_->LoadTexture((basePath + texturePath + "Armer_1.png").c_str());
	TH_hpUI_empty_ = system_->LoadTexture((basePath + texturePath + "Armer_0.png").c_str());

	SH_BGM_ = system_->SoundLoadSE((basePath + soundPath + BGMPath + "Stage.wav").c_str());
	SE_killEnemy_ = system_->SoundLoadSE((basePath + soundPath + SEPath + "killEnemy.wav").c_str());


	//skydome_->CreateDefaultData();
	skydome_ = new Object;
	skydome_->IntObject(system_);
	skydome_->CreateModelData(MH_skydome_);
	skydome_->objectParts_[0].materialConfig->enableLighting = false;
	skydome_->objectParts_[0].materialConfig->useModelTexture = false;
	skydome_->objectParts_[0].materialConfig->textureHandle = TH_buleSkySkydome_;

	underGround_BG_ = new Object;
	underGround_BG_->IntObject(system_);
	underGround_BG_->CreateModelData(MH_underGround_);
	underGround_BG_->objectParts_[0].materialConfig->uvScale = { 500.0f,500.0f,1.0f };
	underGround_BG_->objectParts_[0].materialConfig->textureColor = { 0.3f,0.3f,0.3f,1.0f };
	underGround_BG_->mainPosition.transform.translate.z = +0.5f;

	player_ = new Player(system, { 0.0f, 0.5f, 0 });
	player_->CreateModelData(MH_player_);
	player_->mainPosition.transform.scale = { 0.5f, 0.5f, 0.5f };
	player_->InputBulletList(&bulletList_);

	//Enemy* enemy = new Enemy(system, { 3.0f, 1.0f, 0 });
	//enemy->CreateModelData(MH_enemy_);
	//enemy->mainPosition.transform.scale = { 0.5f,0.5f,0.5f };
	//enemy->objectParts_[0].transform.translate.y = -1.0f;
	//enemyList_.push_back(enemy);

	//MH_enemy_ = system_->SetModelObj("resources/object/player/player.obj");

	for (int i = 0; i < 3; i++) {
		hpUI_[i] = new SimpleSprite;
		hpUI_[i]->IntObject(system);
		hpUI_[i]->CreateDefaultData();
		hpUI_[i]->mainPosition.transform.translate = Vector3(40.0f + i * 80.0f + 40.0f, 60.0f, 0.0f);
		hpUI_[i]->objectParts_[0].materialConfig->textureHandle = TH_hpUI_;
		hpUI_[i]->Update(nullptr);
	}


	targeter_ = new Targeter(system, player_);
	targeter_->IntObject(system_);
	targeter_->CreateModelData(MH_targeter_);
	targeter_->mainPosition.transform.scale = { 0.5f, 0.5f, 0.5f };
	targeter_->objectParts_[0].materialConfig->textureColor = { 0.8f,0.2f,0.2f,1.0f };

	backsign_ = new Object;
	backsign_->IntObject(system_);
	backsign_->CreateModelData(MH_backPoint_);
	backsign_->mainPosition.transform.scale = { 0.5f,0.5f,0.5f };


	/// マップチップの生成
	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("resources/stage/blocks1.csv");
	mapChipField_->SetBlockWidth(1.0f);
	mapChipField_->SetBlockHeight(1.0f);

	/// マップチップの初期化
	GenerateMap();

	/// コリジョンマネージャー初期化
	collisionManager_.InputPlayer(player_);
	collisionManager_.InputMapChip(mapChipField_);
	collisionManager_.InputBulletList(&bulletList_);
	collisionManager_.InputEnemyList(&enemyList_);
	collisionManager_.InputBackPoint(backPoint_);

	/// BGM再生
	system_->SoundPlayBGM(SH_BGM_, 0.7f);
}

Scene1::~Scene1() {

	system_->SoundStop(SH_BGM_);

	delete camera_;
	delete debugCamera_;

	delete targeter_;

	delete player_;
	delete skydome_;
	delete backPoint_;

	delete mapChipField_;


	for (auto& enemy : enemyList_) {
		if (enemy) {
			delete enemy;
		}
	}

	for (auto& bullet : bulletList_) {
		if (bullet) {
			delete bullet;
		}
	}

	for (auto& row : blockObjectList_) {
		for (auto& block : row) {
			if (block) {
				delete block;
			}
		}
	}
}

void Scene1::Update() {
	/// カメラ処理
	CameraPart();

	/// Skydome更新
	skydome_->Update(usingCamera_);

	/// underGround_BG更新
	underGround_BG_->Update(usingCamera_);

	/// player更新
	player_->Update(usingCamera_);

	/// targeter更新
	targeter_->Update(usingCamera_);

	/// Bullet更新
	if (system_->GetMouseIsPush(0)) {
		player_->Shoot(targeter_->GetMouseOnPlane());
	}

	/// Enemy更新
	if (!enemyList_.empty()) {
		for (int i = 0; i < enemyList_.size();) {
			enemyList_[i]->Update(usingCamera_);

			/// 死亡判定
			if (!enemyList_[i]->IsAlive()) {
				delete enemyList_[i];
				enemyList_.erase(enemyList_.begin() + i);
				system_->SoundPlaySE(SE_killEnemy_);
			} else {
				i++;
			}
		}
	}

	/// bullet更新
	if (!bulletList_.empty()) {
		for (int i = 0; i < bulletList_.size();) {
			bulletList_[i]->Update(usingCamera_);

			/// 死亡判定
			if (!bulletList_[i]->IsAlive()) {
				delete bulletList_[i];
				bulletList_.erase(bulletList_.begin() + i);
			} else {
				i++;
			}
		}
	}

	/// backPoint更新
	if (backPoint_) {
		backPoint_->Update(usingCamera_);
		if (backPoint_->GetIsBack()) {

			backsign_->mainPosition.transform.translate = backPoint_->mainPosition.transform.translate + Vector3{ 0,0, -1.0f };
			backsign_->Update(usingCamera_);

			if (system_->GetTriggerOn(DIK_SPACE)) {
				system_->SoundStop(SH_BGM_);
				ChangeNextStage(SceneNum::S_WIN);
			}
		}
	}


	/// HP関連更新
	{
		int hp = player_->GetHP();
		if (hp == 2) hpUI_[2]->objectParts_[0].materialConfig->textureHandle = TH_hpUI_empty_;
		if (hp == 1) hpUI_[1]->objectParts_[0].materialConfig->textureHandle = TH_hpUI_empty_;
		if (hp <= 0) {
			hpUI_[0]->objectParts_[0].materialConfig->textureHandle = TH_hpUI_empty_;
			system_->SoundStop(SH_BGM_);
			ChangeNextStage(SceneNum::S_LOSE);
		}
	}



	//object_->mainPosition.transform.translate = player_->mainPosition.transform.translate + Vector3{1.0f,0,0};
	//object_->Update(usingCamera_);

	/// コリジョン処理
	collisionManager_.Update(system_->GetDeltaTime());

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

	//if (system_->GetTriggerOn(DIK_SPACE)) {
	//	ChangeNextStage(SceneNum::S_Result);
	//}
}


void Scene1::Draw() {

	/// 実体処理
	//system_->DrawModel(&skydome_->objectParts[0].transformationMatrix, &(skydome_->objectParts[0].materialConfig.get()), skydomeModelHandle_);
	skydome_->Draw();
	underGround_BG_->Draw();
	player_->Draw();
	targeter_->Draw();

	//system_->Draw3D(skydome_);
	//system_->Draw3D(player_);
	//system_->Draw3D(model_);

	//object_->Draw();

	/// enemy描画
	for (auto& enemy : enemyList_) {
		enemy->Draw();
	}

	/// bullet描画
	for (auto& bullet : bulletList_) {
		bullet->Draw();
	}

	/// ブロック描画
	Vector3 camPos = usingCamera_->GetTransform().translate;
	const float kBlockDrawRadius = 20.0f; // adjust as needed
	const float kBlockDrawRadiusSq = kBlockDrawRadius * kBlockDrawRadius;
	for (auto& row : blockObjectList_) {
		for (auto& block : row) {
			if (block) {
				Vector3 d = block->mainPosition.transform.translate - camPos;
				float distSq = d.x * d.x + d.y * d.y + d.z * d.z;
				if (distSq <= kBlockDrawRadiusSq) {
					block->Draw();
				}
			}
		}
	}

	if (backPoint_) {
		backPoint_->Draw();
		if (backPoint_->GetIsBack()) {
			backsign_->Draw();
		}
	}

	for (int i = 0; i < 3; i++) {
		hpUI_[i]->Draw();
	}

#ifdef USE_IMGUI
	/// imgui処理
	ImguiPart();
#endif
}

void Scene1::CameraPart() {
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
}

#ifdef USE_IMGUI
void Scene1::ImguiPart() {
	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse", &useDebugCamera);
	ImGui::End();

	{
		int hp = player_->GetHP();
		ImGui::Begin("PlayerPos");
		ImGui::SliderFloat3("Pos", &player_->mainPosition.transform.translate.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Rotate", &player_->mainPosition.transform.rotate.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("TargeterRotate", &targeter_->mainPosition.transform.rotate.x, 0.0f, 5.0f);
		ImGui::Text("Player HP = %d", hp);
		ImGui::End();
	}

	{
		ImGui::Begin("MaterialConfig");
		ImGui::SliderFloat3("uvScale", &underGround_BG_->objectParts_[0].materialConfig->uvScale.x, -50.0f, 50.0f);
		ImGui::End();
	}
}
#endif


void Scene1::GenerateMap() {
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
			MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(j, i);
			if (mapChipType == MapChipType::kEnemy) {
				Enemy* enemy = new Enemy(system_, mapChipField_->GetMapChipPositionByIndex(j, i) /*+ Vector3{ 0.0f,1.0f,0.0f }*/);
				enemy->CreateModelData(MH_enemy_);
				enemy->mainPosition.transform.scale = { 0.5f,0.5f,0.5f };
				enemy->objectParts_[0].transform.translate.y = -1.0f;
				enemyList_.push_back(enemy);
				continue;
			}

			if (mapChipType == MapChipType::kPlayer) {
				player_->mainPosition.transform.translate = mapChipField_->GetMapChipPositionByIndex(j, i) + Vector3{ 0.0f,1.0f,0.0f };
				continue;
			}

			if (mapChipType == MapChipType::kBackPoint) {
				backPoint_ = new BackPoint(system_, mapChipField_->GetMapChipPositionByIndex(j, i));
				continue;
			}

			if (mapChipType != MapChipType::kBlank) {
				blockObjectList_[i][j] = new Object();
				blockObjectList_[i][j]->IntObject(system_);
				blockObjectList_[i][j]->CreateDefaultData();
				blockObjectList_[i][j]->modelHandle_ = config::default_Cube_MeshBufferHandle_;
				blockObjectList_[i][j]->objectParts_[0].materialConfig->useModelTexture = false;
				blockObjectList_[i][j]->mainPosition.transform.translate = mapChipField_->GetMapChipPositionByIndex(j, i);
				if (mapChipType == MapChipType::kDirt) {
					blockObjectList_[i][j]->objectParts_[0].materialConfig->textureHandle = TH_dirt_;
				} else if (mapChipType == MapChipType::kRock) {
					blockObjectList_[i][j]->objectParts_[0].materialConfig->textureHandle = TH_rock_;
				}
			}

		}
	}
}