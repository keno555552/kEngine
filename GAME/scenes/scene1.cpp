#include "Scene1.h"

Scene1::Scene1(kEngine* system) {
	/// =========== システム初期化 ============///
	system_ = system;
	debugCamera_ = system_->CreateDebugCamera();
	camera_ = system_->CreateCamera();
	camera_.lock()->Move({ 0.0f, 0.5f, -10.0f });

	/// =========== リソースロード ============///

	MH_skydome_ = system_->SetModelObj("kEngine/EngineAssets/TemplateResource/object/skydome/skydome.obj");
	MH_player_ = system_->SetModelObj("GAME/resources/Object/player/player.obj");
	MH_sight_ = system_->SetModelObj("GAME/resources/Object/targeter/targeter.obj");
	MH_underGround_ = system_->SetModelObj("GAME/resources/Object/underground_BG/underground_BG.obj");
	MH_bullet_ = system_->SetModelObj("GAME/resources/Object/bullet/bullet.obj");
	MH_object_ = system_->SetModelObj("GAME/resources/Object/pickaxe/pickaxe.obj");
	MH_enemy_ = system_->SetModelObj("GAME/resources/Object/enemy/enemy.obj");
	MH_backPoint_ = system_->SetModelObj("GAME/resources/Object/backSign/backSign.obj");
	
	TH_ReflectTexture_ = system_->LoadTexture("GAME/resources/texture/skyCube/drachenfels_cellar_2k.dds");
	TH_box_ = system_->LoadTexture("kEngine/EngineAssets/texture/testBox.png");
	TH_try_ = system_->LoadTexture("kEngine/EngineAssets/texture/Tryer.png");
	TH_uv_ = system_->LoadTexture("kEngine/EngineAssets/TemplateResource/texture/uvChecker.png");

	TH_dirt_ = system_->LoadTexture("kEngine/EngineAssets/texture/block/dirt.png");
	TH_rock_ = system_->LoadTexture("kEngine/EngineAssets/texture/block/stone.png");

	TH_buleSkySkydome_ = system_->LoadTexture("kEngine/EngineAssets/texture/sky/bluesky.png");

	TH_hpUI_ = system_->LoadTexture("kEngine/EngineAssets/texture/Armer_1.png");
	TH_hpUI_empty_ = system_->LoadTexture("kEngine/EngineAssets/texture/Armer_0.png");

	SH_BGM_ = system_->SoundLoadSE("GAME/resources/sound/BGM/Stage.wav");
	SE_killEnemy_ = system_->SoundLoadSE("GAME/resources/sound/SE/killEnemy.wav");


	//MH_skydome_ = system_->SetModelObj("kEngine/EngineAssets/TemplateResource/object/skydome/skydome.obj");
	//MH_ground_ = system_->SetModelObj("GAME/resources/Object/ground/ground.obj");
	//
	//TH_buleSkySkydome_ = system_->LoadTexture("kEngine/EngineAssets/texture/sky/bluesky.png");
	//TH_title = system_->LoadTexture("kEngine/EngineAssets/texture/title.png");
	//
	//TH_startButton_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/startButton.png");
	//TH_settingButton_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/settingButton.png");
	//TH_quitButton_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/quitButton.png");
	//TH_buttonBack_notSelect_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/buttonNotSelect.png");
	//TH_buttonBack_Select_ = system_->LoadTexture("kEngine/EngineAssets/texture/button/buttonSelect.png");
	//
	//SH_BGM_ = system_->SoundLoadSE("GAME/resources/sound/BGM/Title.wav");
	//SH_Select_ = system_->SoundLoadSE("GAME/resources/sound/SE/menuSelect.wav");
	//SH_Decide_ = system_->SoundLoadSE("GAME/resources/sound/SE/menuChoose.wav");

	system_->GetDrawEngine()->SetEnviromentReflectionTexture(TH_ReflectTexture_);
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
	player_->objectParts_[0].materialConfig->reflectiveStrength = 2.0f;

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
		hpUI_[i]->mainPosition.transform.translate = { 40.0f + i * 80.0f + 40.0f, 60.0f, 0.0f };
		hpUI_[i]->objectParts_[0].materialConfig->textureHandle = TH_hpUI_;
		hpUI_[i]->Update(nullptr);
	}


	sight_ = new Sight(system, player_);
	sight_->IntObject(system_);
	sight_->CreateModelData(MH_sight_);
	//sight_->mainPosition.transform.scale = { 0.5f, 0.5f, 0.5f };
	sight_->mainPosition.transform.scale = { 0.5f, 0.5f, 0.5f };
	sight_->objectParts_[0].materialConfig->textureColor = { 0.8f,0.2f,0.2f,1.0f };

	backSign_ = new Object;
	backSign_->IntObject(system_);
	backSign_->CreateModelData(MH_backPoint_);
	backSign_->mainPosition.transform.scale = { 0.5f,0.5f,0.5f };
	backSign_->objectParts_[0].materialConfig->textureColor = { 1.0f,1.0f,1.0f,0.999f };



	/// マップチップの生成
	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("GAME/resources/Stage/blocks1.csv");
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

	system_->DestroyCamera(camera_);
	system_->DestroyCamera(debugCamera_);

	delete sight_;

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
	skydome_->Update(usingCamera_.lock().get());

	/// underGround_BG更新
	underGround_BG_->Update(usingCamera_.lock().get());

	/// player更新
	player_->Update(usingCamera_.lock().get());

	/// Sight更新
	sight_->Update(usingCamera_.lock().get());

	/// Bullet更新
	if (system_->GetMouseIsPush(0)) {
		player_->Shoot(sight_->GetAngleSightToTarget());
	}

	/// Enemy更新
	if (!enemyList_.empty()) {
		for (int i = 0; i < enemyList_.size();) {
			enemyList_[i]->Update(usingCamera_.lock().get());

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
			bulletList_[i]->Update(usingCamera_.lock().get());

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
		backPoint_->Update(usingCamera_.lock().get());
		if (backPoint_->GetIsBack()) {

			backSign_->mainPosition.transform.translate = backPoint_->mainPosition.transform.translate + Vector3{ 0,0, -1.0f };
			backSign_->Update(nullptr);

			if (system_->GetTriggerOn(DIK_SPACE)) {
				system_->SoundStop(SH_BGM_);
				outcome_ = SceneOutcome::WIN;
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
			outcome_ = SceneOutcome::LOSE;
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
				block->Update(nullptr);
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
	sight_->Draw();

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
	Vector3 camPos = usingCamera_.lock()->GetTransform().translate;
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
			backSign_->Draw();
		}
	}

	for (int i = 0; i < 3; i++) {
		hpUI_[i]->Draw();
	}

#ifdef USE_IMGUI
	/// ImGui処理
	ImGuiPart();
#endif
}

void Scene1::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
		debugCamera_.lock()->MouseControlUpdate();
	} else {
		Transform cameraTransform = CreateDefaultTransform();
		cameraTransform.translate.x = player_->mainPosition.transform.translate.x;
		cameraTransform.translate.y = player_->mainPosition.transform.translate.y + 0.5f;
		cameraTransform.translate.z = player_->mainPosition.transform.translate.z - 15.0f;
		camera_.lock()->SetCamera(cameraTransform);
		usingCamera_ = camera_;
	}
	system_->SetCamera(usingCamera_);
}

#ifdef USE_IMGUI
void Scene1::ImGuiPart() {
	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse", &useDebugCamera);
	ImGui::End();

	{
		int hp = player_->GetHP();
		ImGui::Begin("PlayerPos");
		ImGui::SliderFloat3("Pos", &player_->mainPosition.transform.translate.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Rotate", &player_->mainPosition.transform.rotate.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("TargeterRotate", &sight_->mainPosition.transform.rotate.x, 0.0f, 5.0f);
		ImGui::Text("Player HP = %d", hp);
		ImGui::SliderFloat("Player ReflectiveStrength", &player_->objectParts_[0].materialConfig->reflectiveStrength, 0.0f, 50.0f);
		ImGui::End();
	}

	{
		ImGui::Begin("MaterialConfig");
		ImGui::SliderFloat3("uvScale", &underGround_BG_->objectParts_[0].materialConfig->uvScale.x, -50.0f, 50.0f);
		ImGui::ColorEdit4("textureColor", &backSign_->objectParts_[0].materialConfig->textureColor.x);
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