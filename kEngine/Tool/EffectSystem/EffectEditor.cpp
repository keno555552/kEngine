#include "EffectEditor.h"
#include "DebugDraw.h"
#include "ImguiManager.h"
#include "FileManager\FileManager.h"


EffectEditor::EffectEditor(kEngine* system) {
	/// =========== システム初期化 ============///
	system_ = system;

	light1_ = std::make_unique<Light>();
	light1_->direction = { -0.5f, -1.0f, -0.3f };
	light1_->color = { 1.0f, 1.0f, 1.0f };
	light1_->intensity = 1.0f;
	system_->AddLight(light1_.get());

	light2_ = std::make_unique<Light>();
	light2_->lightingType = LightingType::PointLight;
	light2_->position = { 3.0f, 1.0f, 0.0f };
	light2_->range = 10.0f;
	light2_->color = { 1.0f, 0.2f, 0.2f };
	light2_->intensity = 2.0f;
	system_->AddLight(light2_.get());

	light3_ = std::make_unique<Light>();
	light3_->lightingType = LightingType::SpotLight;
	light3_->position = { 0.0f, 4.0f, 0.0f };	  // 在物體正上方
	light3_->direction = { 0.0f, -1.0f, 0.0f }; // 往下照
	light3_->angle = 0.34906585f;
	light3_->range = 12.0f;
	light3_->color = { 0.3f, 0.3f, 1.0f }; // 藍光
	light3_->intensity = 3.0f;
	system_->AddLight(light3_.get());

	areaLight_ = std::make_unique<AreaLight>();
	areaLight_->lightingType = LightingType::AreaLight;
	areaLight_->position = { 0.0f, 2.0f, 0.0f };
	areaLight_->right = { 1.0f, 0.0f, 0.0f };
	areaLight_->up = { 0.0f, 0.0f, 1.0f };
	areaLight_->width = 1.0f;
	areaLight_->height = 1.0f;
	areaLight_->color = { 1.0f, 0.9f, 0.7f }; // 暖色光
	areaLight_->intensity = 5.0f;
	areaLight_->range = 10.0f;
	system_->AddLight(areaLight_.get());

	areaLight2_ = std::make_unique<AreaLight>();
	areaLight2_->ableLight = false;
	areaLight2_->lightingType = LightingType::AreaLight;
	areaLight2_->position = { 0.0f, 2.0f, 0.0f };
	areaLight2_->right = { 1.0f, 0.0f, 0.0f };
	areaLight2_->up = { 0.0f, 0.0f, 1.0f };
	areaLight2_->width = 1.0f;
	areaLight2_->height = 1.0f;
	areaLight2_->color = { 1.0f, 0.9f, 0.7f }; // 暖色光
	areaLight2_->intensity = 5.0f;
	areaLight2_->range = 10.0f;
	system_->AddLight(areaLight2_.get());

	debugCamera_ = system_->CreateDebugCamera();
	camera_ = system_->CreateCamera();
	usingCamera_ = camera_;
	system_->SetCamera(usingCamera_);

	shootTime.Init0(1, system->GetTimeManager());

	/// =========== リソースロード ============///
	TH_skydomeModelHandle_ = system_->SetModelObj("./kEngine/EngineAssets/TemplateResource/object/skydome/skydome.obj");

	TH_centerAnchorHandle_ = system_->LoadTexture("./kEngine/EngineAssets/TemplateResource/texture/centerPoint.png");

	TH_whiteTextureHandle_ = system_->LoadTexture("./kEngine/EngineAssets/TemplateResource/texture/white5x5.png");
	TH_clicleTextureHandle_ = system_->LoadTexture("./kEngine/EngineAssets/TemplateResource/texture/circle_withAlpha.png");
	TH_effectTextureHandle_ = system_->LoadTexture("./GAME/resources/texture/gradationLine.png");
	// TH_clicleTextureHandle_ = system_->LoadTexture("./kEngine/EngineAssets/TemplateResource/texture/uvChecker.png");

	// BGObject.obj
	// objectHandle_ = system_->SetModelObj("./GAME/Object/Goal/Goal.obj");
	// objectHandle_ = system_->SetModelObj("./kEngine/EngineAssets/Object/charater/charater.obj");

	// ddsTest = system_->LoadTexture("./GAME/resources/texture/skyCube/rostock_laage_airport_4k.dds");
	dds = system_->LoadTexture("./GAME/resources/texture/skyCube/output_skybox.dds");

	system_->GetDrawEngine()->SetEnviromentReflectionTexture(dds);

	ground_ = std::make_unique<Object>();
	ground_->IntObject(system_);
	// ground_->CreateModelData(objectHandle_);
	ground_->mainPosition.transform.translate = { 1.0f, 1.0f, 1.0f };
	ground_->CreateDefaultData();
	ground_->modelHandle_ = config::default_Cube_MeshBufferHandle_;
	ground_->objectParts_[0].materialConfig->textureHandle = TH_whiteTextureHandle_;
	ground_->mainPosition.transform.scale = { 50.0f, 0.1f, 50.0f };
	ground_->mainPosition.transform.translate = { 0.0f, -1.0f, 0.0f };

	centerAnchor_ = std::make_unique<Object>();
	centerAnchor_->IntObject(system_);
	centerAnchor_->CreateDefaultData();
	centerAnchor_->modelHandle_ = config::default_Plane_MeshBufferHandle_;
	centerAnchor_->objectParts_[0].materialConfig->textureHandle = TH_centerAnchorHandle_;
	centerAnchor_->objectParts_[0].materialConfig->textureColor = { 1.0f, 1.0f, 1.0f, 0.999f };
	centerAnchor_->objectParts_[0].materialConfig->enableLighting = false;
	centerAnchor_->isBillboard_ = true;
	centerAnchor_->objectParts_[0].transform.scale = { 0.1f, 0.1f, 0.1f };

	skybox_ = std::make_unique<Object>();
	skybox_->IntObject(system_);
	skybox_->CreateDefaultData();
	skybox_->modelHandle_ = config::default_SkyCube_MeshBufferHandle_;
	skybox_->objectParts_[0].materialConfig->textureHandle = dds;
	skybox_->objectParts_[0].materialConfig->MakePSOEnvironment();
	skybox_->mainPosition.transform.scale = { 200.0f, 200.0f, 200.0f };

	/// =========== パーティクル作る ============///

	HitSpark hitSpark;
	hitSpark.startPosition = { 0.0f, 0.0f, 0.0f };
	hitSpark.objectList[0].objectParts_[0].materialConfig->textureHandle = TH_clicleTextureHandle_;
	TH_particleHandle_ = system_->GetEffectManager()->GetParticleManager()->CreateEmitter(hitSpark, 0);

	HitImpact hitImpact;
	hitImpact.startPosition = { 0.0f, 0.0f, 0.0f };
	hitImpact.objectList[0].objectParts_[0].materialConfig->textureHandle = TH_effectTextureHandle_;
	TH_particleHandle2_ = system_->GetEffectManager()->GetParticleManager()->CreateEmitter(hitImpact, 1);

	HitSpackImpactLink linkData;
	system_->GetEffectManager()->GetParticleManager()->LinkEmitterToEmitter(linkData);

	/// ========== デバッグ用 ============///
	effectDataList_.push_back(Effect_HitSpark());

}

EffectEditor::~EffectEditor() {}

void EffectEditor::Update() {

	SaveLoadPart();
	DeletePart();

	centerAnchor_->Update();
	if (!isPause_) {
		if (isShootNonStop_) {
			if (shootTime.foreverUp()) {
				system_->GetEffectManager()->GetParticleManager()->ShootEmitter(TH_particleHandle_, 1);
			}
		}
	}

	CameraPart();

}

void EffectEditor::Draw() {
	skybox_->Draw();
	ground_->Draw();
	centerAnchor_->Draw();

#ifdef USE_IMGUI
	ImGuiPart();
#endif
}

void EffectEditor::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
		if (auto sp = debugCamera_.lock()) {
			sp->MouseControlUpdate();
		}
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

/// ================= Save/Load関連 ================= ///

void EffectEditor::SaveLoadPart() {
	SavePart();
	LoadPart();
}

void EffectEditor::SavePart() {
	if (!isSave_) return;
	if (currentEffectIndex_ >= 0 && currentEffectIndex_ < static_cast<int>(effectDataList_.size())) {
		switch (saveLoadMode_) {
		case SelectMode::Effect:
			effectAdapter_.SaveEffect(effectDataList_[currentEffectIndex_], saveFilePath_);
			break;
		case SelectMode::Particle:
			if (currentParticleIndex_ >= 0 && currentParticleIndex_ < static_cast<int>(effectDataList_[currentEffectIndex_].prototypes.size())) {
				effectAdapter_.SaveParticle(effectDataList_[currentEffectIndex_].prototypes[currentParticleIndex_], saveFilePath_);
			} else {
				debugText_ += "Error: Invalid currentParticleIndex_ for saving.\n";
			}
			break;
		case SelectMode::Linker:
			if (currentLinkIndex_ >= 0 && currentLinkIndex_ < static_cast<int>(effectDataList_[currentEffectIndex_].links.size())) {
				effectAdapter_.SaveEmitterLink(effectDataList_[currentEffectIndex_].links[currentLinkIndex_], saveFilePath_, effectDataList_[currentEffectIndex_]);
			} else {
				debugText_ += "Error: Invalid currentLinkIndex_ for saving.\n";
			}
			break;
		}
	} else {
		debugText_ += "Error: Invalid currentEffectIndex_ for saving.\n";
	}
	isSave_ = false;
}

void EffectEditor::LoadPart() {
	if (!isLoad_) return;
	isLoad_ = false;
}

void EffectEditor::DeletePart() {
	if (!isDelete_) return;
	if (currentEffectIndex_ >= 0 && currentEffectIndex_ < static_cast<int>(effectDataList_.size())) {
		auto& effect = effectDataList_[currentEffectIndex_];
		switch (saveLoadMode_) {
		case SelectMode::Effect:
			effectDataList_.erase(effectDataList_.begin() + currentEffectIndex_);
			break;
		case SelectMode::Particle:
			if (currentParticleIndex_ >= 0 && currentParticleIndex_ < static_cast<int>(effectDataList_[currentEffectIndex_].prototypes.size())) {
				effect.prototypes.erase(effect.prototypes.begin() + currentParticleIndex_);
			} else {
				debugText_ += "Error: Invalid currentParticleIndex_ for delete.\n";
			}
			break;
		case SelectMode::Linker:
			if (currentLinkIndex_ >= 0 && currentLinkIndex_ < static_cast<int>(effectDataList_[currentEffectIndex_].links.size())) {
				effect.links.erase(effect.links.begin() + currentLinkIndex_);
			} else {
				debugText_ += "Error: Invalid currentLinkIndex_ for delete.\n";
			}
			break;
		}
	} else {
		debugText_ += "Error: Invalid currentEffectIndex_ for delete.\n";
	}
	isDelete_ = false;
}

void EffectEditor::SetNullSavePathToDesktop() {
	if (saveFilePath_.empty()) saveFilePath_ = FileManager::GetDesktopPath();
}

void EffectEditor::SetNullLoadPathToDesktop() {
	if (loadFilePath_.empty()) loadFilePath_ = FileManager::GetDesktopPath();
}


#ifdef USE_IMGUI
void EffectEditor::ImGuiPart() {

	ImGuiIO& io = ImGui::GetIO();
	menuBarHeight_ = ImGui::GetFrameHeight();

	/// ============== メニューバー =============== ///
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {};
			if (ImGui::MenuItem("LoadEffect"))showLoadWindow_ = true;
			if (ImGui::MenuItem("SaveAll"))showSaveWindow_ = true;
			if (ImGui::MenuItem("Exit")) {
				outcome_ = SceneOutcome::EXIT;
				isSceneEnd_ = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	/// ============ 左側のウィンドウ群 ============= ///

	ImGuiLeftMenuBar();

	/// ============ 右側のウィンドウ群 ============= ///

	ImGuiRightMenuBar();

	/// ============ 中央のウィンドウ群 ============= ///
	ImGuiMidWindow();

	/// ============ Popup Window ============= ///
	ImGuiLoadWindow();
	ImGuiSaveWindow();
	ImGuiDoubleCheckWindow();
}


void EffectEditor::ImGuiLeftMenuBar() {
	ImGuiIO& io = ImGui::GetIO();

	float selectorH = menuBarHeight_;
	float lineH = ImGui::GetTextLineHeightWithSpacing();
	float titleH = ImGui::GetFrameHeight();

	float templateLines = 1;
	float templateH = templateLines * lineH + 20.0f + titleH;

	float selectorPosY = menuBarHeight_;
	float templatePosY = io.DisplaySize.y - templateH;

	/// ======== pos計算 =========
	float MapH =
		lineH * 7 +
		ImGui::GetFrameHeight() +
		10.0f +
		titleH;

	float colimenH = (io.DisplaySize.y - selectorH - templateH - titleH * 2) / 3;
	float window2H = 1;
	float window3H = 1;
	float window4H = titleH;

	enum {
		Map = 1 << 0,
		Generator = 1 << 1,
		Variables = 1 << 2,
	};

	int nowState = {
		(isEffectWindowOpen_ ? Map : 0) |
		(isParticleWindowOpen_ ? Generator : 0) |
		(isLinkerWindowOpen_ ? Variables : 0)
	};

	int maxMidWindowCount = 3;
	int windowCount = 0;
	if (nowState & Map)			windowCount++;
	if (nowState & Generator)	windowCount++;
	if (nowState & Variables)	windowCount++;
	int windowLeft = windowCount;

	if (nowState & Map) {
		window2H = colimenH;
		windowLeft--;
		if (windowLeft == 0) window2H += colimenH * (maxMidWindowCount - windowCount);
	}
	if (nowState & Generator) {
		window3H = colimenH;
		if (nowState & Map) window3H += titleH;
		windowLeft--;
		if (windowLeft == 0) window3H += colimenH * (maxMidWindowCount - windowCount);
	}
	if (nowState & Variables) {
		window4H = colimenH;
		if (nowState & Map)				window4H += titleH;
		else if (nowState & Generator)	window4H += titleH;
		windowLeft--;
		if (windowLeft == 0) window4H += colimenH * (maxMidWindowCount - windowCount);
	}

	/// =========================
	/// 1. EffectList
	/// ========================= 
	// ======== pos計算 =========
	ImVec2 w1Pos;
	ImVec2 w1Size;

	ImGui::SetNextWindowPos(ImVec2(0, selectorH));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, window2H));

	isEffectWindowOpen_ = ImGui::Begin("Effect List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float fullW = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float buttonSize = fullW / 5.0f;
	float addW = buttonSize;
	float delW = buttonSize;
	float douW = buttonSize;
	float savW = buttonSize;

	/// New
	if (ImGui::Button("New", ImVec2(addW, 20))) {
		EffectData newEffect;
		newEffect.name = "new_link";
		if (newEffectCounter_ != 0)newEffect.name += "_" + std::to_string(newEffectCounter_);
		newEffectCounter_++;
		effectDataList_.push_back(newEffect);
	}

	ImGui::SameLine();

	/// Double
	if (ImGui::Button("Double", ImVec2(douW, 20))) {
		if (isEffectSelected_) {
			EffectData newEffect = effectDataList_[currentEffectIndex_];
			newEffect.name += "_copy";
			effectDataList_.push_back(newEffect);
		} else {
			debugText_ += "Error: No effect selected for doubling.\n";
		}
	}

	ImGui::SameLine();

	/// Save
	if (ImGui::Button("Save", ImVec2(savW, 20))) {
		if (isEffectSelected_) {
			showSaveWindow_ = true;
			saveLoadMode_ = SelectMode::Effect;
			SetNullSavePathToDesktop();
		} else {
			debugText_ += "Error: No effect selected for saving.\n";
		}
	}

	ImGui::SameLine();

	/// Delete
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(delW, 20))) {
		doubleCheck_ = true;
		saveLoadMode_ = SelectMode::Effect;
	}

	ImGui::PopStyleColor(3);

	// ======== データリスト =========
	ImGui::BeginChild("EffectList", ImVec2(0, window2H - titleH - ImGui::GetFrameHeight() - 20), true);
	for (int i = 0; i < effectDataList_.size(); i++) {
		auto& effect = effectDataList_[i];
		std::string label = effect.name + (effect.isSaved ? "" : " *");

		if (ImGui::Selectable(label.c_str(), currentEffectIndex_ == i)) {
			currentEffectIndex_ = i;
			selectMode_ = SelectMode::Effect;
			//debugText_ += "SelectMode: Effect\n";
		}
	}
	ImGui::EndChild();

	if (currentEffectIndex_ >= 0 && currentEffectIndex_ < effectDataList_.size()) isEffectSelected_ = true;
	else isEffectSelected_ = false;

	// ======== 次のウィンドウに使うデータ =========
	w1Pos = ImGui::GetWindowPos();
	w1Size = ImGui::GetWindowSize();

	ImGui::End();


	/// =========================
	/// 2. Particle List
	/// =========================
	// ======== pos計算 =========
	float window3Y = w1Pos.y + w1Size.y;
	ImVec2 w2Pos;
	ImVec2 w2Size;

	ImGui::SetNextWindowPos(ImVec2(0, window3Y));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, window3H));

	isParticleWindowOpen_ = ImGui::Begin("Particle List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float full2W = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float buttonSize2 = full2W / 5.0f;
	float add2W = buttonSize2;
	float dou2W = buttonSize2;
	float sav2W = buttonSize2;
	float del2W = buttonSize2;

	/// New
	if (ImGui::Button("New", ImVec2(add2W, 20))) {
		if (isEffectSelected_) {
			ParticlePrototype newParticle;
			newParticle.name = "new_link";
			if (newParticleCounter_ != 0)newParticle.name += "_" + std::to_string(newParticleCounter_);
			newParticleCounter_++;
			effectDataList_[currentEffectIndex_].prototypes.push_back(newParticle);
		} else {
			debugText_ += "Error: No effect selected for adding.\n";
		}
	}

	ImGui::SameLine();

	/// Double
	if (ImGui::Button("Double", ImVec2(dou2W, 20))) {
		if (isEffectSelected_) {
			if (currentParticleIndex_ >= 0 && currentParticleIndex_ < static_cast<int>(effectDataList_[currentEffectIndex_].prototypes.size())) {
				ParticlePrototype newPrototype = effectDataList_[currentEffectIndex_].prototypes[currentParticleIndex_];
				newPrototype.name += "_copy";
				effectDataList_[currentEffectIndex_].prototypes.push_back(newPrototype);
			} else {
				debugText_ += "Error: Invalid currentParticleIndex_ for saving.\n";
			}
		} else {
			debugText_ += "Error: No effect selected for doubling.\n";
		}
	}

	ImGui::SameLine();

	/// Save
	if (ImGui::Button("Save", ImVec2(sav2W, 20))) {
		if (isEffectSelected_) {
			if (currentParticleIndex_ >= 0 && currentParticleIndex_ < static_cast<int>(effectDataList_[currentEffectIndex_].prototypes.size())) {
				showSaveWindow_ = true;
				saveLoadMode_ = SelectMode::Particle;
				SetNullSavePathToDesktop();
			} else {
				debugText_ += "Error: Invalid currentParticleIndex_ for saving.\n";
			}
		} else {
			debugText_ += "Error: No effect selected for saving.\n";
		}
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	/// Delete
	if (ImGui::Button("Delete", ImVec2(del2W, 20))) {
		doubleCheck_ = true;
		saveLoadMode_ = SelectMode::Particle;
	}

	ImGui::PopStyleColor(3);

	// ======== データリスト =========


	ImGui::BeginChild("ParticleList", ImVec2(0, window3H - titleH - ImGui::GetFrameHeight() - 20), true);

	if (isEffectSelected_) {
		for (int i = 0; i < effectDataList_[currentEffectIndex_].prototypes.size(); i++) {
			auto& prototype = effectDataList_[currentEffectIndex_].prototypes[i];
			std::string label = prototype.name + (prototype.isSaved ? "" : " *");

			if (ImGui::Selectable(label.c_str(), currentParticleIndex_ == i)) {
				currentParticleIndex_ = i;
				currentLinkIndex_ = -1;
				selectMode_ = SelectMode::Particle;
				//debugText_ += "SelectMode: Particle\n";
			}
		}
	}
	ImGui::EndChild();


	// ======== 次のウィンドウに使うデータ =========
	w2Pos = ImGui::GetWindowPos();
	w2Size = ImGui::GetWindowSize();

	ImGui::End();


	/// =========================
	/// 3. Linker List（獨立視窗）
	/// =========================

	float window4Y = w2Pos.y + w2Size.y;
	ImVec2 w3Pos;
	ImVec2 w3Size;

	ImGui::SetNextWindowPos(ImVec2(0, window4Y));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, window4H));

	isLinkerWindowOpen_ = ImGui::Begin("Linker List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float full3W = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float buttonSize3 = full3W / 5.0f;
	float add3W = buttonSize3;
	float dou3W = buttonSize3;
	float sav3W = buttonSize3;
	float del3W = buttonSize3;

	/// New
	if (ImGui::Button("New", ImVec2(add3W, 20))) {
		if (isEffectSelected_) {
			EmitterLink newLink;
			newLink.name = "new_link";
			if (newLinkCounter_ != 0)newLink.name += "_" + std::to_string(newLinkCounter_);
			newLinkCounter_++;
			effectDataList_[currentEffectIndex_].links.push_back(newLink);
		} else {
			debugText_ += "Error: No effect selected for adding.\n";
		}
	}

	ImGui::SameLine();

	/// Double
	if (ImGui::Button("Double", ImVec2(dou3W, 20))) {
		if (isEffectSelected_) {
			if (currentLinkIndex_ >= 0 && currentLinkIndex_ < static_cast<int>(effectDataList_[currentEffectIndex_].links.size())) {
				EmitterLink newLink = effectDataList_[currentEffectIndex_].links[currentLinkIndex_];
				newLink.name += "_copy";
				effectDataList_[currentEffectIndex_].links.push_back(newLink);
			} else {
				debugText_ += "Error: Invalid currentLinkIndex_ for saving.\n";
			}
		} else {
			debugText_ += "Error: No effect selected for doubling.\n";
		}
	}

	ImGui::SameLine();

	/// Save
	if (ImGui::Button("Save", ImVec2(sav3W, 20))) {
		if (isEffectSelected_) {
			if (currentLinkIndex_ >= 0 && currentLinkIndex_ < static_cast<int>(effectDataList_[currentEffectIndex_].links.size())) {
				showSaveWindow_ = true;
				saveLoadMode_ = SelectMode::Linker;
				SetNullSavePathToDesktop();
			} else {
				debugText_ += "Error: Invalid currentLinkIndex_ for saving.\n";
			}
		} else {
			debugText_ += "Error: No effect selected for saving.\n";
		}
	}

	ImGui::SameLine();

	///

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(del3W, 20))) {
		doubleCheck_ = true;
		saveLoadMode_ = SelectMode::Linker;
	}

	ImGui::PopStyleColor(3);

	int index[2] = { 0, 0 };

	///選択したgeneratorの変数リストを表示する

	ImGui::BeginChild("LinkerList", ImVec2(0, window4H - titleH - ImGui::GetFrameHeight() - 20), true);

	if (isEffectSelected_) {
		for (int i = 0; i < effectDataList_[currentEffectIndex_].links.size(); i++) {
			auto& linker = effectDataList_[currentEffectIndex_].links[i];
			std::string label = linker.name + (linker.isSaved ? "" : " *");

			if (ImGui::Selectable(label.c_str(), currentLinkIndex_ == i)) {
				currentLinkIndex_ = i;
				currentParticleIndex_ = -1;
				selectMode_ = SelectMode::Linker;
				//debugText_ += "SelectMode: Linker\n";
			}
		}
	}
	ImGui::EndChild();

	w3Pos = ImGui::GetWindowPos();
	w3Size = ImGui::GetWindowSize();

	ImGui::End();


	// =========================
	// 4. Engine Info（獨立視窗）
	// =========================

	if (nowState == 0) {
		templatePosY = w3Pos.y + w3Size.y;
		templateH = io.DisplaySize.y - templatePosY;
	}
	ImGui::SetNextWindowPos(ImVec2(0, templatePosY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, templateH));

	ImGui::Begin("Engine Info",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse);

	ImGui::Text("Fps/s: %.2f", system_->GetFPSPerSecond());

	ImGui::End();
}

void EffectEditor::ImGuiRightMenuBar() {

	ImGuiIO& io = ImGui::GetIO();

	ImVec2 w1Pos{};
	ImVec2 w1Size{};

	float selectorH = menuBarHeight_;
	float lineH = ImGui::GetTextLineHeightWithSpacing();
	float titleH = ImGui::GetFrameHeight();

	float lastLineNum = 5;
	float debugLogH = lineH * lastLineNum + 20.0f + titleH;

	float debugLogY = io.DisplaySize.y - debugLogH;

	// =========================
	// 1. Camera
	// =========================
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, menuBarHeight_));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
	ImGui::Begin("Camera",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	rightInspectorWidth_ = ImGui::GetWindowSize().x;

	ImGui::Text("BackTo(0,0):");
	ImGui::SameLine();
	if (ImGui::Button("Click")) {
		debugCamera_.lock()->SetCamera(debugCamera_.lock()->GetDefaultTransform());
	}

	ImGui::Text("Use Debug Camera:");
	ImGui::SameLine();
	ImGui::Checkbox("##Use Debug Camera", &useDebugCamera);

	w1Pos = ImGui::GetWindowPos();
	w1Size = ImGui::GetWindowSize();

	ImGui::End();

	// =========================
	// 2. Shooter Setting
	// =========================
	float window2Y = w1Pos.y + w1Size.y;
	ImVec2 w2Pos;
	ImVec2 w2Size;

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window2Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
	ImGui::Begin("Shooter Setting",
		nullptr,
		ImGuiWindowFlags_NoMove);
	ImGui::Text("Shooter:");
	ImGui::SameLine();
	if (ImGui::Button("Shoot")) {
		system_->GetEffectManager()->GetParticleManager()->ShootEmitter(TH_particleHandle_, 1);
	}

	ImGui::Text("IsShootNonStop:");
	ImGui::SameLine();
	ImGui::Checkbox("##IsShootNonStop:", &isShootNonStop_);

	if (isShootNonStop_) {
		ImGui::Text("ShootTimer:");
		ImGui::SameLine();
		ImGui::Text("0/");
		ImGui::SameLine();
		ImGui::InputFloat("##ShootTime", &shootTime.maxTime_);
	}

	ImGui::Text("Pause:");
	ImGui::SameLine();
	if (ImGui::Button("Pause")) {
		system_->GetEffectManager()->GetParticleManager()->Pause(true);
		isPause_ = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Resume")) {
		system_->GetEffectManager()->GetParticleManager()->Pause(false);
		isPause_ = false;
	}

	w2Pos = ImGui::GetWindowPos();
	w2Size = ImGui::GetWindowSize();

	ImGui::End();

	// =========================
	// 3. Info
	// =========================
	float window3Y = w2Pos.y + w2Size.y;
	ImVec2 w3Pos;
	ImVec2 w3Size;
	w3Size.y = io.DisplaySize.y - window3Y - debugLogH - titleH;

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window3Y));

	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, w3Size.y));
	ImGui::Begin("Variable Details",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::PushItemWidth(150);   // 你想要的寬度
	if (currentEffectIndex_ >= 0 && currentEffectIndex_ < effectDataList_.size()) {
		auto& effectData = effectDataList_[currentEffectIndex_];
		switch (selectMode_) {
		case SelectMode::Effect:
			// Handle Effect-specific UI
			ImGui::Text("Please Select an Particle or Linker");
			break;
		case SelectMode::Particle:
			if (currentParticleIndex_ >= 0 && currentParticleIndex_ < effectData.prototypes.size()) {
				ImGuiParticleShowing(effectData.prototypes[currentParticleIndex_]);
			}
			break;
		case SelectMode::Linker:
			if (currentLinkIndex_ >= 0 && currentLinkIndex_ < effectData.links.size()) {
				ImGuiLinkerShowing(effectData.links[currentLinkIndex_]);
			}
			break;
		default:
			ImGui::Text("No Data Selected");
			break;
		}
	} else {
		ImGui::Text("No Data Selected");
	}
	ImGui::PopItemWidth();

	w3Pos = ImGui::GetWindowPos();
	w3Size = ImGui::GetWindowSize();
	ImGui::End();


	// =========================
	// 4. Inspector Details
	// =========================
	float window4Y = w3Pos.y + w3Size.y;
	float mainBottomY = io.DisplaySize.y;
	float secondHeight = mainBottomY - window4Y;
	if (secondHeight < 0) secondHeight = 0;

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window4Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, secondHeight));
	ImGui::Begin("Debug Log",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Debug Log:");
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		debugText_ = "";
	}
	ImGui::SameLine();
	if (ImGui::Button("AddDummy")) {
		debugText_ += "Dummy debug message\n";
	}
	ImGui::BeginChild("DebugLogChild", ImVec2(0, 0), true);
	ImGui::Text("%s", debugText_.c_str());
	ImGui::SetScrollHereY(1.0f);
	ImGui::EndChild();
	ImGui::End();
}

void EffectEditor::ImGuiMidWindow() {
	ImGuiIO& io = ImGui::GetIO();

	float lineH = ImGui::GetTextLineHeightWithSpacing();
	float titleH = ImGui::GetFrameHeight();;

	// 中間視窗寬度
	float centerPosX = leftInspectorWidth_;
	float centerWidth = io.DisplaySize.x - leftInspectorWidth_ - rightInspectorWidth_;

	// 下のウィンドウの高さ
	int lineCount = 10; // 下のウィンドウに表示する
	if (lineCount < 0) lineCount = 0;

	float bottomH = (lineH * lineCount) + titleH;
	float bottomWindowY;
	if (isMBLWindowOpen_) {
		bottomWindowY = io.DisplaySize.y - bottomH;
	} else {
		bottomWindowY = io.DisplaySize.y - titleH;
	}
	float bottomWindowRY = bottomWindowY;
	float bottomWindowLW = centerWidth;
	float bottomWindowRW = centerWidth / 2;
	if (isMBRWindowOpen_) {
		bottomWindowLW /= 2;
	} else {
		bottomWindowRY -= titleH;
	}

	// ===== Bottom(Left) Window =====
	ImGui::SetNextWindowPos(ImVec2(centerPosX, bottomWindowY));
	ImGui::SetNextWindowSize(ImVec2(bottomWindowLW, bottomH));

	isMBLWindowOpen_ = ImGui::Begin("Drawing Effect Info",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);
	ImGui::Text("Emitter Num: %d", system_->GetEffectManager()->GetParticleManager()->GetEmitterCount());
	ImGui::Text("All Particle Num: %d", system_->GetEffectManager()->GetParticleManager()->GetAllParticleCount());
	ImGui::BeginChild("EmitterList", ImVec2(0, 0), true);

	ImGui::EndChild();

	{
		// ===== Bottom(Right) Window =====
		ImGui::SetNextWindowPos(ImVec2(centerPosX + centerWidth / 2, bottomWindowRY));
		ImGui::SetNextWindowSize(ImVec2(bottomWindowRW, bottomH));
		isMBRWindowOpen_ = ImGui::Begin("Particle Details",
			nullptr,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize);

		ImGui::End();
	}
	ImGui::End();
}

void EffectEditor::ImGuiLoadWindow() {
	if (showLoadWindow_) {
		float lineHeight = ImGui::GetTextLineHeightWithSpacing();
		float padding = ImGui::GetStyle().WindowPadding.y * 2;
		float titleBar = ImGui::GetFrameHeight();
		float fixedHeight = lineHeight * 3 + padding + titleBar;
		ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));
		ImGui::SetNextWindowFocus();
		ImGui::Begin("Load Window", &showLoadWindow_, ImGuiWindowFlags_NoResize);

		InputTextString("Load File Path", loadFilePath_);

		bool isButtonClicked = false;
		if (ImGui::Button("Load")) {
			isButtonClicked = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
		}
		if (isButtonClicked) {
			showLoadWindow_ = false;
		}
		ImGui::End();
	}
}


void EffectEditor::ImGuiSaveWindow() {
	if (showSaveWindow_) {
		float lineHeight = ImGui::GetTextLineHeightWithSpacing();
		float padding = ImGui::GetStyle().WindowPadding.y * 2;
		float titleBar = ImGui::GetFrameHeight();
		float fixedHeight = lineHeight * 3 + padding + titleBar;
		ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));
		ImGui::SetNextWindowFocus();
		ImGui::Begin("Save Window", &showSaveWindow_, ImGuiWindowFlags_NoResize);

		InputTextString("Save File Path", saveFilePath_);

		bool isButtonClicked = false;
		if (ImGui::Button("Save")) {
			isButtonClicked = true;
			isSave_ = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
			saveLoadMode_ = SelectMode::None;
		}
		if (isButtonClicked) {
			showSaveWindow_ = false;
		}
		ImGui::End();
	}
}

void EffectEditor::ImGuiDoubleCheckWindow() {
	if (doubleCheck_) {
		float lineHeight = ImGui::GetTextLineHeightWithSpacing();
		float padding = ImGui::GetStyle().WindowPadding.y * 2;
		float titleBar = ImGui::GetFrameHeight();
		float fixedHeight = lineHeight * 3 + padding + titleBar;
		ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));
		ImGui::SetNextWindowFocus();
		ImGui::Begin("Double Check Window", &doubleCheck_, ImGuiWindowFlags_NoResize);

		ImGui::Text("Are you sure you want to delete this item?");

		bool isButtonClicked = false;
		if (ImGui::Button("Delete")) {
			isButtonClicked = true;
			isDelete_ = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
			saveLoadMode_ = SelectMode::None;
		}
		if (isButtonClicked) {
			doubleCheck_ = false;
		}
		ImGui::End();
	}
}

void EffectEditor::ImGuiEffectShowing() {}

void EffectEditor::ImGuiParticleShowing(ParticlePrototype& particle) {

	InputTextString("Linker Name", particle.name);

	if (ImGui::CollapsingHeader("Lifetime")) {
		InputFloat("lifetime", particle.lifetime);
		InputFloat("lifetimeRandomness", particle.lifetimeRandomness);
	}

	if (ImGui::CollapsingHeader("Position")) {
		InputVector3("Start", particle.startPosition);
		InputVector3("StartRandom", particle.startPositionRandom);
	}

	if (ImGui::CollapsingHeader("Velocity")) {
		InputVector3("StartVelocity", particle.startVelocity);
		InputVector3("StartRandom", particle.startVelocityRandom);
		InputVector3("StartSpeedRandom", particle.startSpeedRandom);
	}

	if (ImGui::CollapsingHeader("Scale")) {
		InputVector3("Start", particle.startScale);
		InputVector3("End", particle.endScale);
		InputVector3("StartRandom", particle.startScaleRandom);
		InputVector3("EndRandom", particle.endScaleRandom);
		ImGui::Checkbox("isConstant", &particle.isConstantScale);
	}

	if (ImGui::CollapsingHeader("Rotation")) {
		InputVector3("Start", particle.startRotation);
		InputVector3("End", particle.endRotation);
		InputVector3("StartRandom", particle.startRotationRandom);
		InputVector3("EndRandom", particle.endRotationRandom);
		ImGui::Checkbox("isConstant", &particle.isConstantRotation);
	}

	if (ImGui::CollapsingHeader("Color")) {
		InputVector4("Start", particle.startColor);
		InputVector4("End", particle.endColor);
		InputVector4("StartRandom", particle.startColorRandom);
		InputVector4("EndRandom", particle.endColorRandom);
		ImGui::Checkbox("isConstant", &particle.isConstantColor);
	}

	if (ImGui::CollapsingHeader("Movement")) {
		InputVector3("Gravity", particle.gravity);
		InputFloat("damping", particle.damping);
		InputFloat("dampingRandom", particle.dampingRandom);
	}

	if (ImGui::CollapsingHeader("EmittingSettings")) {
		InputFloat("Rate", particle.emitRate);
		InputFloat("IntervalRandom", particle.emitIntervalRandom);
		InputInt("BurstCount", particle.burstCount);
		InputFloat("EmitNumRandom", particle.emitNumRandom);
	}

}

void EffectEditor::ImGuiLinkerShowing(EmitterLink& linker) {

	InputTextString("Linker Name", linker.name);
	InputTextString("Source Name", linker.sourceName);
	InputTextString("Target Name", linker.targetName);
	InputInt("Emit Count", linker.emitCount);
	InputFloat("Delay Time", linker.delayTime);

	const char* linkModeItems[] = { "PerBurst", "PerParticle" };
	EnumCombo("LinkMode", linker.linkMode, linkModeItems, IM_ARRAYSIZE(linkModeItems));

	const char* linkFollowItems[] = { "Particle", "Emitter" };
	EnumCombo("LinkFollow", linker.linkFollow, linkFollowItems, IM_ARRAYSIZE(linkFollowItems));

	const char* emitterTimingItems[] = { "SourceEmit", "TimeUp", "SourceEnd" };
	EnumCombo("Emitter Timing", linker.emitterTiming, emitterTimingItems, IM_ARRAYSIZE(emitterTimingItems));

}

// --- 專用 Callback ---
static int InputTextCallback(ImGuiInputTextCallbackData* data) {
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
		auto* str = reinterpret_cast<std::string*>(data->UserData);
		str->resize(data->BufTextLen);
		data->Buf = const_cast<char*>(str->c_str());
	}
	return 0;
}

bool EffectEditor::InputTextString(const char* label, std::string& str, ImGuiInputTextFlags flags) {
	flags |= ImGuiInputTextFlags_CallbackResize;

	ImGui::Text("%s:", label);
	ImGui::SameLine();

	std::string newLabel = std::string("##") + label; // 隐藏标签

	if (str.capacity() == 0)
		str.reserve(16);

	return ImGui::InputText(
		newLabel.c_str(),
		const_cast<char*>(str.c_str()),
		str.capacity() + 1,
		flags,
		InputTextCallback,
		&str);
}

bool EffectEditor::InputBigTextString(const char* label, std::string& str, ImGuiInputTextFlags flags) {
	flags |= ImGuiInputTextFlags_CallbackResize;

	ImGui::Text("%s:", label);

	std::string newLabel = std::string("##") + label; // 隐藏标签

	if (str.capacity() == 0)
		str.reserve(16);

	// 取得視窗內容區域大小（扣掉 Text 的高度）
	ImVec2 avail = ImGui::GetContentRegionAvail();

	// 下方 padding
	avail.y -= ImGui::GetTextLineHeight();

	// 字一個分の左右 padding を追加
	float padX = ImGui::GetFontSize();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padX, ImGui::GetStyle().FramePadding.y));

	bool result = ImGui::InputTextMultiline(
		newLabel.c_str(),
		const_cast<char*>(str.c_str()),
		str.capacity() + 1,
		avail,
		flags,
		InputTextCallback,
		&str);

	ImGui::PopStyleVar();

	// 下方 padding
	ImGui::Dummy(ImVec2(0, ImGui::GetTextLineHeight()));

	return result;
}

void EffectEditor::InputInt(const char* label, int& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputInt(newLabel.c_str(), &value);
}

void EffectEditor::InputFloat(const char* label, float& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat(newLabel.c_str(), &value);
}

void EffectEditor::InputVector3(const char* label, Vector3& vec) {

	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat3(newLabel.c_str(), &vec.x);
}
void EffectEditor::InputVector4(const char* label, Vector4& vec) {

	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat4(newLabel.c_str(), &vec.x);
}
#endif

/// ImDrawList* draw = ImGui::GetWindowDrawList();
/// ImVec2 p = ImGui::GetCursorScreenPos();
/// draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 100, p.y + 50), IM_COL32(0, 255, 0, 255));