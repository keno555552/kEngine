#include "EffectEditor.h"
#include "DebugDraw.h"
#include "ImguiManager.h"

EffectEditor::EffectEditor(kEngine *system)
{
	/// =========== システム初期化 ============///
	system_ = system;

	light1_ = std::make_unique<Light>();
	light1_->direction = {-0.5f, -1.0f, -0.3f};
	light1_->color = {1.0f, 1.0f, 1.0f};
	light1_->intensity = 1.0f;
	system_->AddLight(light1_.get());

	light2_ = std::make_unique<Light>();
	light2_->lightingType = LightingType::PointLight;
	light2_->position = {3.0f, 1.0f, 0.0f};
	light2_->range = 10.0f;
	light2_->color = {1.0f, 0.2f, 0.2f};
	light2_->intensity = 2.0f;
	system_->AddLight(light2_.get());

	light3_ = std::make_unique<Light>();
	light3_->lightingType = LightingType::SpotLight;
	light3_->position = {0.0f, 4.0f, 0.0f};	  // 在物體正上方
	light3_->direction = {0.0f, -1.0f, 0.0f}; // 往下照
	light3_->angle = 0.34906585f;
	light3_->range = 12.0f;
	light3_->color = {0.3f, 0.3f, 1.0f}; // 藍光
	light3_->intensity = 3.0f;
	system_->AddLight(light3_.get());

	areaLight_ = std::make_unique<AreaLight>();
	areaLight_->lightingType = LightingType::AreaLight;
	areaLight_->position = {0.0f, 2.0f, 0.0f};
	areaLight_->right = {1.0f, 0.0f, 0.0f};
	areaLight_->up = {0.0f, 0.0f, 1.0f};
	areaLight_->width = 1.0f;
	areaLight_->height = 1.0f;
	areaLight_->color = {1.0f, 0.9f, 0.7f}; // 暖色光
	areaLight_->intensity = 5.0f;
	areaLight_->range = 10.0f;
	system_->AddLight(areaLight_.get());

	areaLight2_ = std::make_unique<AreaLight>();
	areaLight2_->ableLight = false;
	areaLight2_->lightingType = LightingType::AreaLight;
	areaLight2_->position = {0.0f, 2.0f, 0.0f};
	areaLight2_->right = {1.0f, 0.0f, 0.0f};
	areaLight2_->up = {0.0f, 0.0f, 1.0f};
	areaLight2_->width = 1.0f;
	areaLight2_->height = 1.0f;
	areaLight2_->color = {1.0f, 0.9f, 0.7f}; // 暖色光
	areaLight2_->intensity = 5.0f;
	areaLight2_->range = 10.0f;
	system_->AddLight(areaLight2_.get());

	debugCamera_ = system_->CreateDebugCamera();
	camera_ = system_->CreateCamera();
	usingCamera_ = camera_;
	system_->SetCamera(usingCamera_);

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
	ground_->mainPosition.transform.translate = {1.0f, 1.0f, 1.0f};
	ground_->CreateDefaultData();
	ground_->modelHandle_ = config::default_Cube_MeshBufferHandle_;
	ground_->objectParts_[0].materialConfig->textureHandle = TH_whiteTextureHandle_;
	ground_->mainPosition.transform.scale = {50.0f, 0.1f, 50.0f};
	ground_->mainPosition.transform.translate = {0.0f, -1.0f, 0.0f};

	centerAnchor_ = std::make_unique<Object>();
	centerAnchor_->IntObject(system_);
	centerAnchor_->CreateDefaultData();
	centerAnchor_->modelHandle_ = config::default_Plane_MeshBufferHandle_;
	centerAnchor_->objectParts_[0].materialConfig->textureHandle = TH_centerAnchorHandle_;
	centerAnchor_->objectParts_[0].materialConfig->textureColor = {1.0f, 1.0f, 1.0f, 0.999f};
	centerAnchor_->objectParts_[0].materialConfig->enableLighting = false;
	centerAnchor_->isBillboard_ = true;
	centerAnchor_->objectParts_[0].transform.scale = {0.1f, 0.1f, 0.1f};

	skybox_ = std::make_unique<Object>();
	skybox_->IntObject(system_);
	skybox_->CreateDefaultData();
	skybox_->modelHandle_ = config::default_SkyCube_MeshBufferHandle_;
	skybox_->objectParts_[0].materialConfig->textureHandle = dds;
	skybox_->objectParts_[0].materialConfig->MakePSOEnvironment();
	skybox_->mainPosition.transform.scale = {200.0f, 200.0f, 200.0f};

	/// =========== パーティクル作る ============///

	// HitSpark hitSpark;
	// hitSpark.startPosition = { 0.0f, 0.0f, 0.0f };
	// hitSpark.objectList[0].objectParts_[0].materialConfig->textureHandle = clicleTextureHandle_;
	// particleHandle_ = system_->GetEffectManager()->GetParticleManager()->CreateEmitter(hitSpark, 0);
	//
	// HitImpact hitImpact;
	// hitImpact.startPosition = { 0.0f, 0.0f, 0.0f };
	// hitImpact.objectList[0].objectParts_[0].materialConfig->textureHandle = effectTextureHandle_;
	// particleHandle2_ = system_->GetEffectManager()->GetParticleManager()->CreateEmitter(hitImpact, 1);
	//
	// HitSpackImpactLink linkData;
	// linkData.sourceId = particleHandle_;
	// linkData.targetId = particleHandle2_;
	// system_->GetEffectManager()->GetParticleManager()->LinkEmitterToEmitter(linkData);
}

EffectEditor::~EffectEditor() {}

void EffectEditor::Update()
{
	centerAnchor_->Update();

	CameraPart();

}

void EffectEditor::Draw()
{
	skybox_->Draw();
	ground_->Draw();
	centerAnchor_->Draw();

	ImGuiPart();
}

void EffectEditor::ImGuiPart()
{

	ImGuiIO &io = ImGui::GetIO();
	menuBarHeight_ = ImGui::GetFrameHeight();

	/// ============== メニューバー =============== ///
	if (ImGui::BeginMainMenuBar())
	{

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New")){};
			if (ImGui::MenuItem("Load"))showLoadWindow_ = true;
			if (ImGui::MenuItem("Save"))showSaveWindow_ = true;
			if (ImGui::MenuItem("Exit"))
			{
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

	/// ============ Popup Window ============= ///
	ImGuiLoadWindow();
	ImGuiSaveWindow();
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

void EffectEditor::ImGuiLeftMenuBar() {
	ImGuiIO& io = ImGui::GetIO();

	float splitterSize = 4.0f;

	// 左側容器 Window（關鍵）
	ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight_));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, io.DisplaySize.y - menuBarHeight_));
	ImGui::Begin("LeftPanel",
		nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse);

	// ======================= Splitter ========================= //
	ImGui::SetCursorPos(ImVec2(leftInspectorWidth_ - splitterSize, 0));
	ImGui::InvisibleButton("##Splitter", ImVec2(splitterSize, io.DisplaySize.y - menuBarHeight_));

	if (ImGui::IsItemActive())
		leftInspectorWidth_ += ImGui::GetIO().MouseDelta.x;

	// 限制最小寬度
	if (leftInspectorWidth_ < 150) leftInspectorWidth_ = 150;
	if (leftInspectorWidth_ > io.DisplaySize.x - 150) leftInspectorWidth_ = io.DisplaySize.x - 150;

	float leftWidth = leftInspectorWidth_;

	float screenH = io.DisplaySize.y;
	float effectWindowHeight = ImGui::GetTextLineHeightWithSpacing() * 5 + 40.0f;

	// ======================= Effects ========================= //
	ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight_));
	ImGui::SetNextWindowSize(ImVec2(leftWidth, effectWindowHeight));
	ImGui::Begin("Effects", nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize);

	if (ImGui::Button("Add Effect", ImVec2(leftWidth - 20, 30))) {}
	ImGui::Separator();

	ImGui::BeginChild("EffectListChild", ImVec2(0, effectWindowHeight - 60), true);
	for (int i = 0; i < 5; i++)
		ImGui::Selectable(("Effect " + std::to_string(i)).c_str());
	ImGui::EndChild();

	ImGui::End();


	// ======================= Particles ========================= //
	float particleWindowY = menuBarHeight_ + effectWindowHeight;

	int templateLines = 3;
	float templateHeight = templateLines * ImGui::GetTextLineHeightWithSpacing() + 40.0f;

	float particleWindowHeight = screenH - particleWindowY - templateHeight;
	if (particleWindowHeight < 0) particleWindowHeight = 0;

	ImGui::SetNextWindowPos(ImVec2(0, particleWindowY));
	ImGui::SetNextWindowSize(ImVec2(leftWidth, particleWindowHeight));
	ImGui::Begin("Particles", nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize);

	if (ImGui::Button("Add Particle", ImVec2(leftWidth - 20, 30))) {}
	ImGui::Separator();

	ImGui::BeginChild("ParticleListChild", ImVec2(0, particleWindowHeight - 60), true);
	for (int i = 0; i < 10; i++)
		ImGui::Selectable(("Particle " + std::to_string(i)).c_str());
	ImGui::EndChild();

	ImGui::End();

	// ======================= Template Info ========================= //
	float templateWindowY = particleWindowY + particleWindowHeight;

	ImGui::SetNextWindowPos(ImVec2(0, templateWindowY));
	ImGui::SetNextWindowSize(ImVec2(leftWidth, templateHeight));
	ImGui::Begin("Template Info", nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize);

	ImGui::Text("Fps/s:%.2f", system_->GetFPSPerSecond());

	ImGui::End();

	ImGui::End(); // LeftPanel
}

void EffectEditor::ImGuiRightMenuBar() {
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 w1Pos{};
	ImVec2 w1Size{};

	// ===== Camera =====
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, menuBarHeight_));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
	ImGui::Begin("Camera",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_AlwaysAutoResize);

	rightInspectorWidth_ = ImGui::GetWindowSize().x;

	ImGui::Text("BackTo(0,0):");
	ImGui::SameLine();
	if (ImGui::Button("Click")) {
		debugCamera_.lock()->SetCamera(debugCamera_.lock()->GetDefaultTransform());
	}

	ImGui::Checkbox("Use Debug Camera", &useDebugCamera);

	w1Pos = ImGui::GetWindowPos();
	w1Size = ImGui::GetWindowSize();

	ImGui::End();


	// ===== Window 2 =====
	float window2Y = w1Pos.y + w1Size.y;
	ImVec2 w2Pos;
	ImVec2 w2Size;

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window2Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
	ImGui::Begin("Window 2",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Second window content");
	ImGui::Text("Item 1");
	ImGui::Text("Item 2");
	ImGui::Text("Item 3");

	w2Pos = ImGui::GetWindowPos();
	w2Size = ImGui::GetWindowSize();

	ImGui::End();


	// ===== Inspector Details =====
	float window3Y = w2Pos.y + w2Size.y;
	float mainBottomY = io.DisplaySize.y;
	float secondHeight = mainBottomY - window3Y;
	if (secondHeight < 0) secondHeight = 0;

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window3Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, secondHeight));
	ImGui::Begin("Inspector Details",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_AlwaysAutoResize);

	InputBigTextString("Codeing Text", codeingText_);

	ImGui::End();
}

void EffectEditor::ImGuiLoadWindow()
{
	if (showLoadWindow_)
	{
		float lineHeight = ImGui::GetTextLineHeightWithSpacing();
		float padding = ImGui::GetStyle().WindowPadding.y * 2;
		float titleBar = ImGui::GetFrameHeight();
		float fixedHeight = lineHeight * 3 + padding + titleBar;
		ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));
		ImGui::SetNextWindowFocus();
		ImGui::Begin("Load Window", &showLoadWindow_, ImGuiWindowFlags_NoResize);

		InputTextString("Load File Path", loadFilePath_);

		bool isButtonClicked = false;
		if (ImGui::Button("Load"))
		{
			isButtonClicked = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
		{
			isButtonClicked = true;
		}
		if (isButtonClicked)
		{
			showLoadWindow_ = false;
		}
		ImGui::End();
	}
}


void EffectEditor::ImGuiSaveWindow()
{
	if (showSaveWindow_)
	{
		float lineHeight = ImGui::GetTextLineHeightWithSpacing();
		float padding = ImGui::GetStyle().WindowPadding.y * 2;
		float titleBar = ImGui::GetFrameHeight();
		float fixedHeight = lineHeight * 3 + padding + titleBar;
		ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));
		ImGui::SetNextWindowFocus();
		ImGui::Begin("Save Window", &showSaveWindow_, ImGuiWindowFlags_NoResize);

		InputTextString("Save File Path", saveFilePath_);

		bool isButtonClicked = false;
		if (ImGui::Button("Save"))
		{
			isButtonClicked = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close"))
		{
			isButtonClicked = true;
		}
		if (isButtonClicked)
		{
			showSaveWindow_ = false;
		}
		ImGui::End();
	}
}

// --- 專用 Callback ---
static int InputTextCallback(ImGuiInputTextCallbackData *data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		auto *str = reinterpret_cast<std::string *>(data->UserData);
		str->resize(data->BufTextLen);
		data->Buf = const_cast<char *>(str->c_str());
	}
	return 0;
}

bool EffectEditor::InputTextString(const char *label, std::string &str, ImGuiInputTextFlags flags)
{
	flags |= ImGuiInputTextFlags_CallbackResize;

	ImGui::Text("%s:", label);
	ImGui::SameLine();

	std::string newLabel = std::string("##") + label; // 隐藏标签

	if (str.capacity() == 0)
		str.reserve(16);

	return ImGui::InputText(
		newLabel.c_str(),
		const_cast<char *>(str.c_str()),
		str.capacity() + 1,
		flags,
		InputTextCallback,
		&str);
}

bool EffectEditor::InputBigTextString(const char *label, std::string &str, ImGuiInputTextFlags flags)
{
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
		const_cast<char *>(str.c_str()),
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

/// ImDrawList* draw = ImGui::GetWindowDrawList();
/// ImVec2 p = ImGui::GetCursorScreenPos();
/// draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 100, p.y + 50), IM_COL32(0, 255, 0, 255));