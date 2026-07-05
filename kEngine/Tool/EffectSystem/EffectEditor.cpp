#include "EffectEditor.h"
#include "DebugDraw.h"
#include "ImguiManager.h"

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
	linkData.sourceId = TH_particleHandle_;
	linkData.targetId = TH_particleHandle2_;
	system_->GetEffectManager()->GetParticleManager()->LinkEmitterToEmitter(linkData);
}

EffectEditor::~EffectEditor() {}

void EffectEditor::Update() {
	centerAnchor_->Update();
	if(!isPause_){
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

#ifdef USE_IMGUI
void EffectEditor::ImGuiPart() {

	ImGuiIO& io = ImGui::GetIO();
	menuBarHeight_ = ImGui::GetFrameHeight();

	/// ============== メニューバー =============== ///
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {};
			if (ImGui::MenuItem("Load"))showLoadWindow_ = true;
			if (ImGui::MenuItem("Save"))showSaveWindow_ = true;
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

	// ======== pos計算 =========
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

	// =========================
	// 1. EffectList
	// =========================
	ImVec2 w1Pos;
	ImVec2 w1Size;

	ImGui::SetNextWindowPos(ImVec2(0, selectorH));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, window2H));

	//ImGui::SetNextWindowCollapsed(false, ImGuiCond_FirstUseEver);
	isEffectWindowOpen_ = ImGui::Begin("Effect List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float fullW = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float addW = fullW * 0.7f;
	float delW = fullW * 0.3f;

	if (ImGui::Button("Add Effect", ImVec2(addW, 20))) {
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(delW, 20))) {
	}

	ImGui::PopStyleColor(3);

	ImGui::BeginChild("EffectList", ImVec2(0, window2H - titleH - ImGui::GetFrameHeight() - 20), true);
	//for (int i = 0; i < mapData_.size(); i++) {
	//	auto& m = mapData_[i];
	//	std::string label = m.tileMapData.name + (m.isSaved ? "" : " *");
	//
	//	if (ImGui::Selectable(label.c_str(), selectedMap_ == i)) {
	//		selectedMap_ = i;
	//	}
	//}
	ImGui::EndChild();

	w1Pos = ImGui::GetWindowPos();
	w1Size = ImGui::GetWindowSize();

	ImGui::End();


	// =========================
	// 2. Particle List
	// =========================

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
	float add2W = full2W * 0.7f;
	float del2W = full2W * 0.3f;

	if (ImGui::Button("Add Particle", ImVec2(add2W, 20))) {
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(del2W, 20))) {
	}

	ImGui::PopStyleColor(3);

	ImGui::BeginChild("ParticleList", ImVec2(0, window3H - titleH - ImGui::GetFrameHeight() - 20), true);
	//for (int i = 0; i < 10; i++)
	//	ImGui::Selectable(("Particle " + std::to_string(i)).c_str());
	ImGui::EndChild();

	w2Pos = ImGui::GetWindowPos();
	w2Size = ImGui::GetWindowSize();

	ImGui::End();


	// =========================
	// 3. Linker List（獨立視窗）
	// =========================

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
	float add3W = full3W * 0.7f;
	float del3W = full3W * 0.3f;

	if (ImGui::Button("Add Link", ImVec2(add3W, 20))) {
		//showNewVariablesWindow_ = true;
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(del3W, 20))) {
	}

	ImGui::PopStyleColor(3);

	ImGui::BeginChild("LinkerList", ImVec2(0, window4H - titleH - ImGui::GetFrameHeight() - 20), true);
	int index[2] = { 0, 0 };

	///選択したgeneratorの変数リストを表示する
	int unitIndex = 0;
	//for (auto& variableUnit : generatorVariables_[selectedGenerator_].unitLists) {
	//
	//	int varIndex = 0;
	//	for (auto& variable : variableUnit.units) {
	//
	//		std::string typeName = "[" + variableUnit.unitName + "]";
	//		std::string label = typeName + variable.name;
	//
	//		bool isSelected =
	//			selectedVariable_.first == unitIndex &&
	//			selectedVariable_.second == varIndex;
	//
	//		if (ImGui::Selectable(label.c_str(), isSelected)) {
	//			selectedVariable_ = { unitIndex, varIndex };
	//		}
	//
	//		varIndex++;
	//	}
	//	unitIndex++;
	//}
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
	//if (selectedMap_ != -1) {
	//	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
	//	ImGui::Begin("Map Generator Info",
	//		nullptr,
	//		ImGuiWindowFlags_NoMove |
	//		ImGuiWindowFlags_AlwaysAutoResize);
	//
	//	if (selectedMap_ != -1) {
	//		MapInfo& selectedMapInfo = mapData_[selectedMap_];
	//		ImGui::Text("Selected Map: %s", selectedMapInfo.tileMapData.name.c_str());
	//		int sizeX = (selectedMapInfo.tileMapData.Row.empty()) ? 0 : (int)selectedMapInfo.tileMapData.Row[0].size();
	//		int sizeY = (selectedMapInfo.tileMapData.Row.empty()) ? 0 : (int)selectedMapInfo.tileMapData.Row.size();
	//		ImGui::Text("Map Size: %d x %d", sizeX, sizeY);
	//	}
	//
	//	w2Pos = ImGui::GetWindowPos();
	//	w2Size = ImGui::GetWindowSize();
	//} else {

	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, w3Size.y));
	ImGui::Begin("Variable Details",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);
	w3Pos = ImGui::GetWindowPos();
	w3Size = ImGui::GetWindowSize();

	ImGui::Text("Dummy");
	//}

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
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
		}
		if (isButtonClicked) {
			showSaveWindow_ = false;
		}
		ImGui::End();
	}
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
#endif

/// ImDrawList* draw = ImGui::GetWindowDrawList();
/// ImVec2 p = ImGui::GetCursorScreenPos();
/// draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 100, p.y + 50), IM_COL32(0, 255, 0, 255));