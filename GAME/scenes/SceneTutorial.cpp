#include "SceneTutorial.h"

SceneTutorial::SceneTutorial(kEngine* system) {
	/// =========== システム初期化 ============///
	system_ = system;
	debugCamera_ = system_->CreateDebugCamera();
	camera_ = system_->CreateCamera();
	camera_.lock()->Move({ 0.0f, 0.5f, -10.0f });

	phaseChangeTimer_ = new Timer();

	/// =========== 後処理初期化 ==============///

	ppp_GlitchScreen_ = new PPP_GlitchScreen(system_);
	noiseCommand_ = ppp_GlitchScreen_->GetNoiseCommand();
	colorGradingCommand_ = ppp_GlitchScreen_->GetColorGradingCommand();
	blurCommand_ = ppp_GlitchScreen_->GetBlurCommand();
	vignetteCommand_ = ppp_GlitchScreen_->GetVignetteCommand();

	circleCommand_ = new RenderCommand();
	circleCommand_->vignetteCenter = { 0.5f, 0.5f };
	circleCommand_->vignetteRadius = circleRadius;
	circleCommand_->vignetteSoftness = circleSoftness;
	circleCommand_->vignetteIntensity = circleCloseIntensity;

	circleHandle_ = system_->GetPostProcessRunner()->SetPassVignette(0, *circleCommand_);

	/// =========== リソースロード ============///

	MH_skydome_ = system_->SetModelObj("kEngine/EngineAssets/TemplateResource/object/skydome/skydome.obj");
	MH_player_ = system_->SetModelObj("GAME/resources/Object/player/player.obj");
	MH_underGround_ = system_->SetModelObj("GAME/resources/Object/underground_BG/underground_BG.obj");
	MH_bullet_ = system_->SetModelObj("GAME/resources/Object/bullet/bullet.obj");
	MH_object_ = system_->SetModelObj("GAME/resources/Object/pickaxe/pickaxe.obj");
	MH_enemy_ = system_->SetModelObj("GAME/resources/Object/enemy/enemy.obj");
	MH_backPoint_ = system_->SetModelObj("GAME/resources/Object/backSign/backSign.obj");

	//TH_ReflectTexture_ = system_->LoadTexture("GAME/resources/texture/skyCube/drachenfels_cellar_2k.dds");
	TH_ReflectTexture_ = system_->LoadTexture("./GAME/resources/texture/skyCube/output_skybox.dds");
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

	backSign_ = new Object;
	backSign_->IntObject(system_);
	backSign_->CreateModelData(MH_backPoint_);
	backSign_->mainPosition.transform.scale = { 0.5f,0.5f,0.5f };
	backSign_->objectParts_[0].materialConfig->textureColor = { 1.0f,1.0f,1.0f,0.999f };

	/// マップチップの生成
	mapChipManager_ = std::make_unique<MapChipManager>(system_);
	const bool loaded = mapChipManager_->AddLayer("Main", "GAME/resources/Stage/1YearGame/Maps/tutorial.csv");
	assert(loaded && "tutorial が読めない（csv か MapData）");

	/// マップチップの初期化
	RegisterMapSpawner();
	GenerateMap();

	/// コリジョンマネージャー初期化
	collisionManager_.InputPlayer(player_);
	collisionManager_.InputMapChip(mapChipManager_.get());
	collisionManager_.InputBulletList(&bulletList_);
	collisionManager_.InputEnemyList(&enemyList_);
	collisionManager_.InputBackPoint(backPoint_);

	/// BGM再生
	//system_->SoundPlayBGM(SH_BGM_, 0.7f);
}

SceneTutorial::~SceneTutorial() {

	system_->SoundStop(SH_BGM_);

	system_->DestroyCamera(camera_);
	system_->DestroyCamera(debugCamera_);

	delete player_;
	delete skydome_;
	delete backPoint_;


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

	ppp_GlitchScreen_->OffAllPass();
	delete ppp_GlitchScreen_;
}

void SceneTutorial::Update() {

	/// 後処理更新
	PhaseUpdate();
	//ppp_GlitchScreen_->Update();

	/// カメラ処理
	CameraPart();

	/// Skydome更新
	skydome_->Update(usingCamera_.lock().get());

	/// underGround_BG更新
	underGround_BG_->Update(usingCamera_.lock().get());

	/// player更新
	if (system_->GetIsPush(DIK_A)) player_->MoveLeft();
	if (system_->GetIsPush(DIK_D)) player_->MoveRight();
	if (system_->GetIsPush(DIK_W)) player_->MoveJump();
	player_->Update(usingCamera_.lock().get());

	/// Bullet更新
	if (system_->GetMouseIsPush(0)) {
		player_->Shoot();
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

	/// Sign更新
	if (!signList_.empty()) {
		for (int i = 0; i < signList_.size();i++) {
			signList_[i]->Update(usingCamera_.lock().get());
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
				isWin_ = true;
				phase_ = ScenePhase::TRANSITION;
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
			isSceneEnd_ = true;
		}
	}



	//object_->mainPosition.transform.translate = player_->mainPosition.transform.translate + Vector3{1.0f,0,0};
	//object_->Update(usingCamera_);

	/// コリジョン処理
	collisionManager_.Update(system_->GetDeltaTime());

	/// ブロック更新
	mapBlocks_.Update();

	if (system_->GetTriggerOn(DIK_0)) {
		if (useDebugCamera)useDebugCamera = false;
		else useDebugCamera = true;
	}

	//if (system_->GetTriggerOn(DIK_SPACE)) {
	//	ChangeNextStage(SceneNum::S_Result);
	//}
}


void SceneTutorial::Draw() {

	/// 実体処理
	//system_->DrawModel(&skydome_->objectParts[0].transformationMatrix, &(skydome_->objectParts[0].materialConfig.get()), skydomeModelHandle_);
	skydome_->Draw();
	underGround_BG_->Draw();
	player_->Draw();

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

	/// sign描画
	for (auto& sign : signList_) {
		sign->Draw();
	}

	/// ブロック描画
	Vector3 camPos = usingCamera_.lock()->GetTransform().translate;
	const float kBlockDrawRadius = 20.0f; // adjust as needed
	const float kBlockDrawRadiusSq = kBlockDrawRadius * kBlockDrawRadius;
	mapBlocks_.Draw(camPos, kBlockDrawRadius);

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

void SceneTutorial::PhaseUpdate() {

	switch (phase_) {
	case ScenePhase::PREPARE:

		phase_ = ScenePhase::ENTRY;

	case ScenePhase::ENTRY:

		/// タイマー初期化
		if (phaseChangeTimer_->maxTime_ == 0.0f) {
			phaseChangeTimer_->Init0(entryTime_, system_->GetTimeManager());
		}
		/// 線方補間
		circleCommand_->vignetteIntensity = phaseChangeTimer_->linearity(circleCloseIntensity, circleOpenIntensity);
		/// circleに代入
		system_->GetPostProcessRunner()->SetPassCommand(0, circleHandle_, *circleCommand_);

		/// カンター行動
		if (phaseChangeTimer_->ToMix()) {
			phase_ = ScenePhase::PROGRESS;
		}

		break;

	case ScenePhase::PROGRESS:
		break;
	case ScenePhase::TRANSITION:
		/// タイマー初期化
		if (phaseChangeTimer_->GetIsMax()) {
			phaseChangeTimer_->Init0(entryTime_, system_->GetTimeManager());
		}
		/// 線方補間
		circleCommand_->vignetteIntensity = phaseChangeTimer_->linearity(circleOpenIntensity, circleCloseIntensity);
		/// circleに代入
		system_->GetPostProcessRunner()->SetPassCommand(0, circleHandle_, *circleCommand_);

		/// カンター行動
		if (phaseChangeTimer_->ToMix()) {
			outcome_ = SceneOutcome::WIN;
		}
		break;
	}
}

void SceneTutorial::CameraPart() {
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
void SceneTutorial::ImGuiPart() {
	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse", &useDebugCamera);
	ImGui::End();

	{
		int hp = player_->GetHP();
		ImGui::Begin("PlayerPos");
		ImGui::SliderFloat3("Pos", &player_->mainPosition.transform.translate.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Rotate", &player_->mainPosition.transform.rotate.x, -1.0f, 1.0f);
		// ImGui::SliderFloat3("TargeterRotate", &player_->GetAimPosition, 0.0f, 5.0f);
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

	{
		{

			ImGui::Begin("RenderCommand");
			ImGui::Text("ColorGuard");

			ImGui::Combo("Guard Type", &currentGuardType, "noiseCommand_\0ColorGradingCommand_\0BlurCommand_\0VignetteCommand_\0");

			RenderCommand* renderCommand = nullptr;
			switch (currentGuardType) {
			case 0:
				renderCommand = noiseCommand_;
				break;
			case 1:
				renderCommand = colorGradingCommand_;
				break;
			case 2:
				renderCommand = blurCommand_;
				break;
			case 3:
				renderCommand = vignetteCommand_;
				break;
			}


			ImGui::SliderFloat3("Guard Color", &renderCommand->guardColor.x, 0, 1);
			ImGui::SliderFloat("Guard Amount", &renderCommand->guardAmount, 0, 1);
			ImGui::Text("Vignette");
			ImGui::SliderFloat2("Vignette Center", &renderCommand->vignetteCenter.x, 0, 1);
			ImGui::SliderFloat("Vignette Radius", &renderCommand->vignetteRadius, 0, 1);
			ImGui::SliderFloat("Vignette Softness", &renderCommand->vignetteSoftness, 0, 1);
			ImGui::SliderFloat("Vignette Intensity", &renderCommand->vignetteIntensity, 0, 100);
			ImGui::ColorEdit4("Vignette Color", &renderCommand->vignetteColor.x);
			ImGui::Text("Blur");
			// BlurType 対応する文字列の配列
			static const char* blurType[] = {
			"Box",
			"Custom"
			};
			// 今の BlendModeTypeをとる
			int currentBlurType = static_cast<int>(renderCommand->blurType);
			// 選択肢
			if (ImGui::Combo("BlurType", &currentBlurType, blurType, IM_ARRAYSIZE(blurType))) {
				renderCommand->blurType = static_cast<KernelType>(currentBlurType);
			}
			ImGui::SliderInt("Kernel Size", &renderCommand->blurKernelSize, 1, 7);
			ImGui::Text("Outline");
			ImGui::SliderFloat("Outline Depth Threshold", &renderCommand->outlineDepthThreshold, 0.01f, 0.1f);
			ImGui::ColorEdit4("Outline Color", &renderCommand->outlineColor.x);
			ImGui::SliderFloat("Radial CenterX", &renderCommand->blurRadialCenter.x, 0.0f, 1280.0f);
			ImGui::SliderFloat("Radial CenterY", &renderCommand->blurRadialCenter.y, 0.0f, 720.0f);
			ImGui::SliderInt("Radial Sample Size", &renderCommand->blurRadialSampleSize, 1, 64);
			ImGui::SliderFloat("Radial Strength", &renderCommand->blurRadialStrength, 0.0f, 1.0f);
			ImGui::SliderFloat("Dissolve Threshold", &renderCommand->dissolveThreshold, 0.0f, 1.0f);
			ImGui::SliderFloat("Dissolve Edge Width", &renderCommand->dissolveEdgeWidth, 0.0f, 1.0f);
			ImGui::ColorEdit3("Dissolve Edge Color", &renderCommand->dissolveEdgeColor.x);
			ImGui::Combo("Random Noise Type", reinterpret_cast<int*>(&renderCommand->randomNoiseType), "None\0WhiteNoise\0");
			ImGui::SliderFloat("Random Noise Amount", &renderCommand->randomNoiseAmount, 0.0f, 1.0f);
			ImGui::SliderFloat("Random Noise Time", &renderCommand->randomNoiseTime, 0.0f, 100.0f);


			ImGui::End();
			ImGui::Begin("Effect");
			ImGui::Text("Particle num: %d", system_->GetEffectManager()->GetParticleManager()->GetAllParticleCount());
			ImGui::End();

			ImGui::Begin("Circle");
			ImGui::SliderFloat2("Circle Center", &circleCommand_->vignetteCenter.x, 0, 1);
			ImGui::SliderFloat("Circle Radius", &circleCommand_->vignetteRadius, 0, 1);
			ImGui::SliderFloat("Circle Softness", &circleCommand_->vignetteSoftness, 0, 1);
			ImGui::SliderFloat("Circle Intensity", &circleCommand_->vignetteIntensity, 0, 100);
			ImGui::ColorEdit4("Circle Color", &circleCommand_->vignetteColor.x);
			ImGui::End();
		}
	}
}
#endif

void SceneTutorial::RegisterMapSpawner() {
	/// 名前 → 保存先 ← 生成関数
	mapChipManager_->ClearRegistry();
	mapChipManager_->Register(playerName_,         &player_,    this, &SceneTutorial::CreatePlayer);
	mapChipManager_->Register(enemyName_,          &enemyList_, this, &SceneTutorial::CreateEnemy);
	mapChipManager_->Register(shipSpawnPointName_, &backPoint_, this, &SceneTutorial::CreateBackPoint);
	mapChipManager_->Register(signName_,           &signList_,  this, &SceneTutorial::CreateSign);
	mapChipManager_->SetDefault(this, &SceneTutorial::SpawnDefault);
}

void SceneTutorial::GenerateMap() {
	mapBlocks_.Init(mapChipManager_.get(), "Main");
	mapChipManager_->Generate("Main");
}

/// ============================== 生成関数 ============================== ///

Player* SceneTutorial::CreatePlayer(const MapReader::SpawnInfo& s) {
	Player* player = new Player(system_, s.position);
	player->CreateModelData(MH_player_);
	player->mainPosition.transform.scale = { 0.5f, 0.5f, 0.5f };
	player->InputBulletList(&bulletList_);
	player->objectParts_[0].materialConfig->reflectiveStrength = 1.3f;
	return player;
}

Enemy* SceneTutorial::CreateEnemy(const MapReader::SpawnInfo& s) {
	Enemy* enemy = new Enemy(system_, s.position);
	enemy->CreateModelData(MH_enemy_);
	enemy->mainPosition.transform.scale = { 0.5f,0.5f,0.5f };
	enemy->objectParts_[0].transform.translate.y = -1.0f;
	return enemy;
}

BackPoint* SceneTutorial::CreateBackPoint(const MapReader::SpawnInfo& s) {
	return new BackPoint(system_, s.position);
}

Sign* SceneTutorial::CreateSign(const MapReader::SpawnInfo& s) {
	return new Sign(system_, s.position);
}

/// 登録に無い物：直接タイルはブロック、それ以外は登録漏れとして記録
void SceneTutorial::SpawnDefault(const MapReader::SpawnInfo& s) {
	if (!mapBlocks_.OnSpawn(s)) {
		unhandledMapUnits_.insert(s.unit ? s.unit->name : std::to_string(s.id));
	}
}