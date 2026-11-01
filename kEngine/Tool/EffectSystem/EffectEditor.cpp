#include "EffectEditor.h"
#include "DebugDraw.h"
#include "ImguiManager.h"
#include "FileManager\FileManager.h"
#include "externals/imgui/imgui_internal.h"

namespace {
	/// ウィンドウ名（Begin と DockBuilderDockWindow で同じ文字列を使うため、一か所で定義する）
	constexpr const char* kWinEffectList = "Effect List";
	constexpr const char* kWinParticleList = "Particle List";
	constexpr const char* kWinLinkerList = "Linker List";
	constexpr const char* kWinEngineInfo = "Engine Info";
	constexpr const char* kWinCamera = "Camera";
	constexpr const char* kWinShooter = "Shooter Setting";
	constexpr const char* kWinDetails = "Variable Details";
	constexpr const char* kWinDebugLog = "Debug Log";
	constexpr const char* kWinDrawingInfo = "Drawing Effect Info";
	constexpr const char* kWinParticleInfo = "Particle Details";
}


EffectEditor::EffectEditor(kEngine* system) : effectAdapter_(system) {
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

	/// ---------- Save All ----------
	if (isSaveAll_) {
		isSaveAll_ = false;
		for (auto& effect : effectDataList_) {
			if (!effect.isSaved) SaveEffectData(effect, false);
		}
	}

	/// ---------- 単体のSave ----------
	if (!isSave_) return;

	bool forceDialog = isSaveAs_;
	SelectMode mode = saveLoadMode_;
	isSave_ = false;
	isSaveAs_ = false;
	saveLoadMode_ = SelectMode::None;

	if (!isEffectSelected_) {
		debugText_ += "Error: No effect selected for saving.\n";
		return;
	}
	auto& effect = effectDataList_[currentEffectIndex_];

	switch (mode) {
	case SelectMode::Effect:
		SaveEffectData(effect, forceDialog);
		break;

	case SelectMode::Particle:
		if (currentParticleIndex_ >= 0 && currentParticleIndex_ < (int)effect.prototypes.size()) {
			SaveParticleData(effect.prototypes[currentParticleIndex_], forceDialog);
		} else {
			debugText_ += "Error: Invalid currentParticleIndex_ for saving.\n";
		}
		break;

	case SelectMode::Linker:
		if (currentLinkIndex_ >= 0 && currentLinkIndex_ < (int)effect.links.size()) {
			SaveLinkData(effect.links[currentLinkIndex_], effect, forceDialog);
		} else {
			debugText_ += "Error: Invalid currentLinkIndex_ for saving.\n";
		}
		break;

	default:
		break;
	}
}

void EffectEditor::LoadPart() {
	if (!isLoad_) return;
	isLoad_ = false;
}

bool EffectEditor::SaveEffectData(EffectData& effect, bool forceDialog) {
	std::string path = effect.savePath;
	if (forceDialog || path.empty()) {
		path = AskSavePath(effect.savePath, effect.name);
		if (path.empty()) return false;   // キャンセル
	}

	std::string error;
	if (!effectAdapter_.SaveEffect(effect, path, &error)) {
		debugText_ += "Error: " + error + "\n";
		return false;
	}

	/// Effectごと保存したので、中身も全部保存済み扱い
	effect.savePath = path;
	effect.isSaved = true;
	for (auto& p : effect.prototypes) p.isSaved = true;
	for (auto& l : effect.links) l.isSaved = true;

	debugText_ += "Saved Effect: " + path + "\n";
	return true;
}

bool EffectEditor::SaveParticleData(ParticlePrototype& particle, bool forceDialog) {
	std::string path = particle.savePath;
	if (forceDialog || path.empty()) {
		path = AskSavePath(particle.savePath, particle.name);
		if (path.empty()) return false;
	}

	std::string error;
	if (!effectAdapter_.SaveParticle(particle, path, &error)) {
		debugText_ += "Error: " + error + "\n";
		return false;
	}

	particle.savePath = path;
	particle.isSaved = true;
	debugText_ += "Saved Particle: " + path + "\n";
	return true;
}

bool EffectEditor::SaveLinkData(EmitterLink& link, const EffectData& owner, bool forceDialog) {
	/// source / target が同じEffect内に存在するかチェック（保存は止めない、警告だけ）
	auto exists = [&](const std::string& name) {
		for (const auto& p : owner.prototypes) if (p.name == name) return true;
		return false;
		};
	if (!exists(link.sourceName)) debugText_ += "Warning: source \"" + link.sourceName + "\" not found in effect.\n";
	if (!exists(link.targetName)) debugText_ += "Warning: target \"" + link.targetName + "\" not found in effect.\n";

	std::string path = link.savePath;
	if (forceDialog || path.empty()) {
		path = AskSavePath(link.savePath, link.name);
		if (path.empty()) return false;
	}

	std::string error;
	if (!effectAdapter_.SaveEmitterLink(link, path, &error)) {
		debugText_ += "Error: " + error + "\n";
		return false;
	}

	link.savePath = path;
	link.isSaved = true;
	debugText_ += "Saved Link: " + path + "\n";
	return true;
}

std::string EffectEditor::AskSavePath(const std::string& lastPath, const std::string& name) {
	/// 前回の保存先があればそれ、なければ名前を初期ファイル名にする
	std::string defaultName = lastPath.empty()
		? FileManager::ToSafeFileName(name) + ".json"
		: lastPath;

	return FileManager::SaveFileDialog(
		"JSON (*.json)\0*.json\0All Files (*.*)\0*.*\0",
		"json",
		defaultName,
		"Save");
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

void EffectEditor::SetNullLoadPathToDesktop() {
	if (loadFilePath_.empty()) loadFilePath_ = FileManager::GetDesktopPath();
}


#ifdef USE_IMGUI
void EffectEditor::ImGuiPart() {

	/// ============== メニューバー =============== ///
	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New")) {};
		if (ImGui::MenuItem("LoadEffect")) showLoadWindow_ = true;
		if (ImGui::MenuItem("Save Effect As...", nullptr, false, isEffectSelected_)) {
			isSave_ = true;
			isSaveAs_ = true;
			saveLoadMode_ = SelectMode::Effect;
		}
		if (ImGui::MenuItem("Save All")) isSaveAll_ = true;
		if (ImGui::MenuItem("Exit")) {
			outcome_ = SceneOutcome::EXIT;
			isSceneEnd_ = true;
		}
		ImGui::EndMenu();
	}

	/// ============ ドックスペース（各ウィンドウより先に） ============= ///
	ImGuiDockSpace();

	ImGuiLeftMenuBar();
	ImGuiRightMenuBar();
	ImGuiMidWindow();

	/// ============ Popup Window ============= ///
	ImGuiLoadWindow();
	ImGuiDoubleCheckWindow();
}

void EffectEditor::ImGuiDockSpace() {
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	/// メインウィンドウ（メニューバーを除く）全体を覆う、透明なホストウィンドウ
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags hostFlags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("##EffectEditorHost", nullptr, hostFlags);
	ImGui::PopStyleVar(3);

	ImGuiID dockId = ImGui::GetID("EffectEditorDockSpace");

	/// imgui.ini に配置が無い（初回）か、Reset Layout のときだけ作る
	if (ImGui::DockBuilderGetNode(dockId) == nullptr || isResetLayout_) {
		BuildDefaultLayout(dockId, viewport->WorkSize);
		isResetLayout_ = false;
	}

	/// 真ん中は透明（後ろの3Dシーンが見える）、真ん中にはドックさせない
	ImGui::DockSpace(dockId, ImVec2(0.0f, 0.0f),
		ImGuiDockNodeFlags_PassthruCentralNode |
		ImGuiDockNodeFlags_NoDockingOverCentralNode);

	ImGui::End();
}

void EffectEditor::BuildDefaultLayout(ImGuiID dockId, const ImVec2& size) {
	ImGui::DockBuilderRemoveNode(dockId);
	ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockId, size);   // Splitの前に必ずサイズを決める

	/// --- 大きく分ける：左 / 右 / 下 / 真ん中 ---
	ImGuiID center = dockId;
	ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.20f, nullptr, &center);
	ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);
	ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, nullptr, &center);

	/// --- 左列：Effect / Particle / Linker / EngineInfo ---
	ImGuiID leftInfo = ImGui::DockBuilderSplitNode(left, ImGuiDir_Down, 0.08f, nullptr, &left);
	ImGuiID leftEffect = ImGui::DockBuilderSplitNode(left, ImGuiDir_Up, 0.33f, nullptr, &left);
	ImGuiID leftParticle = ImGui::DockBuilderSplitNode(left, ImGuiDir_Up, 0.50f, nullptr, &left);
	ImGuiID leftLinker = left;

	/// --- 右列：Camera / Shooter / Details / DebugLog ---
	ImGuiID rightLog = ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.25f, nullptr, &right);
	ImGuiID rightCamera = ImGui::DockBuilderSplitNode(right, ImGuiDir_Up, 0.12f, nullptr, &right);
	ImGuiID rightShooter = ImGui::DockBuilderSplitNode(right, ImGuiDir_Up, 0.20f, nullptr, &right);
	ImGuiID rightDetails = right;

	/// --- 下：左右半分 ---
	ImGuiID bottomRight = ImGui::DockBuilderSplitNode(bottom, ImGuiDir_Right, 0.50f, nullptr, &bottom);

	/// --- ウィンドウを入れる ---
	ImGui::DockBuilderDockWindow(kWinEffectList, leftEffect);
	ImGui::DockBuilderDockWindow(kWinParticleList, leftParticle);
	ImGui::DockBuilderDockWindow(kWinLinkerList, leftLinker);
	ImGui::DockBuilderDockWindow(kWinEngineInfo, leftInfo);
	ImGui::DockBuilderDockWindow(kWinCamera, rightCamera);
	ImGui::DockBuilderDockWindow(kWinShooter, rightShooter);
	ImGui::DockBuilderDockWindow(kWinDetails, rightDetails);
	ImGui::DockBuilderDockWindow(kWinDebugLog, rightLog);
	ImGui::DockBuilderDockWindow(kWinDrawingInfo, bottom);
	ImGui::DockBuilderDockWindow(kWinParticleInfo, bottomRight);

	ImGui::DockBuilderFinish(dockId);
}

float EffectEditor::CalcButtonWidth(int count) {
	float spacing = ImGui::GetStyle().ItemSpacing.x;
	float avail = ImGui::GetContentRegionAvail().x;
	return (avail - spacing * (count - 1)) / count;
}

void EffectEditor::ImGuiLeftMenuBar() {

	/// =========================
	/// 1. Effect List
	/// =========================
	if (ImGui::Begin(kWinEffectList)) {
		float buttonW = CalcButtonWidth(4);

		/// New
		if (ImGui::Button("New", ImVec2(buttonW, 0))) {
			EffectData newEffect;
			newEffect.name = "new_effect";
			if (newEffectCounter_ != 0) newEffect.name += "_" + std::to_string(newEffectCounter_);
			newEffectCounter_++;
			effectDataList_.push_back(newEffect);
		}
		ImGui::SameLine();

		/// Double
		if (ImGui::Button("Double", ImVec2(buttonW, 0))) {
			if (isEffectSelected_) {
				EffectData newEffect = effectDataList_[currentEffectIndex_];
				newEffect.name += "_copy";
				newEffect.savePath.clear();
				newEffect.isSaved = false;
				effectDataList_.push_back(newEffect);
			} else {
				debugText_ += "Error: No effect selected for doubling.\n";
			}
		}
		ImGui::SameLine();

		/// Save
		if (ImGui::Button("Save", ImVec2(buttonW, 0))) {
			if (isEffectSelected_) {
				isSave_ = true;
				saveLoadMode_ = SelectMode::Effect;
			} else {
				debugText_ += "Error: No effect selected for saving.\n";
			}
		}
		ImGui::SameLine();

		/// Delete
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));
		if (ImGui::Button("Delete", ImVec2(buttonW, 0))) {
			doubleCheck_ = true;
			saveLoadMode_ = SelectMode::Effect;
		}
		ImGui::PopStyleColor(3);

		/// データリスト（残りの高さを全部使う）
		ImGui::BeginChild("EffectList", ImVec2(0, 0), true);
		for (int i = 0; i < (int)effectDataList_.size(); i++) {
			auto& effect = effectDataList_[i];
			std::string label = effect.name + (effect.isSaved ? "" : " *");
			ImGui::PushID(i);   // 同名でもIDが被らないように
			if (ImGui::Selectable(label.c_str(), currentEffectIndex_ == i)) {
				if (currentEffectIndex_ != i) {
					/// 別のEffectに切り替えたら下の選択はリセット
					currentParticleIndex_ = -1;
					currentLinkIndex_ = -1;
				}
				currentEffectIndex_ = i;
				selectMode_ = SelectMode::Effect;
			}
			ImGui::PopID();
		}
		ImGui::EndChild();
	}
	ImGui::End();

	/// タブが隠れていても選択状態は更新する
	isEffectSelected_ = (currentEffectIndex_ >= 0 && currentEffectIndex_ < (int)effectDataList_.size());


	/// =========================
	/// 2. Particle List
	/// =========================
	if (ImGui::Begin(kWinParticleList)) {
		float buttonW = CalcButtonWidth(4);

		/// New
		if (ImGui::Button("New", ImVec2(buttonW, 0))) {
			if (isEffectSelected_) {
				ParticlePrototype newParticle;
				newParticle.name = "new_particle";
				if (newParticleCounter_ != 0) newParticle.name += "_" + std::to_string(newParticleCounter_);
				newParticleCounter_++;
				effectDataList_[currentEffectIndex_].prototypes.push_back(newParticle);
			} else {
				debugText_ += "Error: No effect selected for adding.\n";
			}
		}
		ImGui::SameLine();

		/// Double
		if (ImGui::Button("Double", ImVec2(buttonW, 0))) {
			if (isEffectSelected_) {
				auto& prototypes = effectDataList_[currentEffectIndex_].prototypes;
				if (currentParticleIndex_ >= 0 && currentParticleIndex_ < (int)prototypes.size()) {
					ParticlePrototype newPrototype = prototypes[currentParticleIndex_];
					newPrototype.name += "_copy";
					prototypes.push_back(newPrototype);
				} else {
					debugText_ += "Error: Invalid currentParticleIndex_ for doubling.\n";
				}
			} else {
				debugText_ += "Error: No effect selected for doubling.\n";
			}
		}
		ImGui::SameLine();

		/// Save
		if (ImGui::Button("Save", ImVec2(buttonW, 0))) {
			if (isEffectSelected_) {
				isSave_ = true;
				saveLoadMode_ = SelectMode::Effect;
			} else {
				debugText_ += "Error: No effect selected for saving.\n";
			}
		}
		ImGui::SameLine();

		/// Delete
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));
		if (ImGui::Button("Delete", ImVec2(buttonW, 0))) {
			doubleCheck_ = true;
			saveLoadMode_ = SelectMode::Particle;
		}
		ImGui::PopStyleColor(3);

		/// データリスト
		ImGui::BeginChild("ParticleList", ImVec2(0, 0), true);
		if (isEffectSelected_) {
			auto& prototypes = effectDataList_[currentEffectIndex_].prototypes;
			for (int i = 0; i < (int)prototypes.size(); i++) {
				auto& prototype = prototypes[i];
				std::string label = prototype.name + (prototype.isSaved ? "" : " *");
				ImGui::PushID(i);
				if (ImGui::Selectable(label.c_str(), currentParticleIndex_ == i)) {
					currentParticleIndex_ = i;
					currentLinkIndex_ = -1;
					selectMode_ = SelectMode::Particle;
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();


	/// =========================
	/// 3. Linker List
	/// =========================
	if (ImGui::Begin(kWinLinkerList)) {
		float buttonW = CalcButtonWidth(4);

		/// New
		if (ImGui::Button("New", ImVec2(buttonW, 0))) {
			if (isEffectSelected_) {
				EmitterLink newLink;
				newLink.name = "new_link";
				if (newLinkCounter_ != 0) newLink.name += "_" + std::to_string(newLinkCounter_);
				newLinkCounter_++;
				effectDataList_[currentEffectIndex_].links.push_back(newLink);
			} else {
				debugText_ += "Error: No effect selected for adding.\n";
			}
		}
		ImGui::SameLine();

		/// Double
		if (ImGui::Button("Double", ImVec2(buttonW, 0))) {
			if (isEffectSelected_) {
				auto& links = effectDataList_[currentEffectIndex_].links;
				if (currentLinkIndex_ >= 0 && currentLinkIndex_ < (int)links.size()) {
					EmitterLink newLink = links[currentLinkIndex_];
					newLink.name += "_copy";
					links.push_back(newLink);
				} else {
					debugText_ += "Error: Invalid currentLinkIndex_ for doubling.\n";
				}
			} else {
				debugText_ += "Error: No effect selected for doubling.\n";
			}
		}
		ImGui::SameLine();

		/// Save
		if (ImGui::Button("Save", ImVec2(buttonW, 0))) {
			if (isEffectSelected_) {
				isSave_ = true;
				saveLoadMode_ = SelectMode::Effect;
			} else {
				debugText_ += "Error: No effect selected for saving.\n";
			}
		}
		ImGui::SameLine();

		/// Delete
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));
		if (ImGui::Button("Delete", ImVec2(buttonW, 0))) {
			doubleCheck_ = true;
			saveLoadMode_ = SelectMode::Linker;
		}
		ImGui::PopStyleColor(3);

		/// データリスト
		ImGui::BeginChild("LinkerList", ImVec2(0, 0), true);
		if (isEffectSelected_) {
			auto& links = effectDataList_[currentEffectIndex_].links;
			for (int i = 0; i < (int)links.size(); i++) {
				auto& linker = links[i];
				std::string label = linker.name + (linker.isSaved ? "" : " *");
				ImGui::PushID(i);
				if (ImGui::Selectable(label.c_str(), currentLinkIndex_ == i)) {
					currentLinkIndex_ = i;
					currentParticleIndex_ = -1;
					selectMode_ = SelectMode::Linker;
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();


	/// =========================
	/// 4. Engine Info
	/// =========================
	if (ImGui::Begin(kWinEngineInfo)) {
		ImGui::Text("Fps/s: %.2f", system_->GetFPSPerSecond());
	}
	ImGui::End();
}

void EffectEditor::ImGuiRightMenuBar() {

	/// =========================
	/// 1. Camera
	/// =========================
	if (ImGui::Begin(kWinCamera)) {
		ImGui::Text("BackTo(0,0):");
		ImGui::SameLine();
		if (ImGui::Button("Click")) {
			if (auto cam = debugCamera_.lock()) {
				cam->SetCamera(cam->GetDefaultTransform());
			}
		}

		ImGui::Text("Use Debug Camera:");
		ImGui::SameLine();
		ImGui::Checkbox("##Use Debug Camera", &useDebugCamera);
	}
	ImGui::End();

	/// =========================
	/// 2. Shooter Setting
	/// =========================
	if (ImGui::Begin(kWinShooter)) {
		auto* pm = system_->GetEffectManager()->GetParticleManager();

		ImGui::Text("Shooter:");
		ImGui::SameLine();
		if (ImGui::Button("Shoot")) {
			pm->ShootEmitter(TH_particleHandle_, 1);
		}

		ImGui::Text("IsShootNonStop:");
		ImGui::SameLine();
		ImGui::Checkbox("##IsShootNonStop", &isShootNonStop_);

		if (isShootNonStop_) {
			ImGui::Text("ShootTimer: 0/");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputFloat("##ShootTime", &shootTime.maxTime_);
		}

		ImGui::Text("Pause:");
		ImGui::SameLine();
		if (ImGui::Button("Pause")) {
			pm->Pause(true);
			isPause_ = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Resume")) {
			pm->Pause(false);
			isPause_ = false;
		}
	}
	ImGui::End();

	/// =========================
	/// 3. Variable Details
	/// =========================
	if (ImGui::Begin(kWinDetails)) {
		ImGui::PushItemWidth(-FLT_MIN);   // 入力欄をパネルの右端まで伸ばす

		if (isEffectSelected_) {
			auto& effectData = effectDataList_[currentEffectIndex_];
			switch (selectMode_) {
			case SelectMode::Effect:
				ImGui::Text("Please Select a Particle or Linker");
				break;
			case SelectMode::Particle:
				if (currentParticleIndex_ >= 0 && currentParticleIndex_ < (int)effectData.prototypes.size()) {
					ImGuiParticleShowing(effectData.prototypes[currentParticleIndex_]);
				}
				break;
			case SelectMode::Linker:
				if (currentLinkIndex_ >= 0 && currentLinkIndex_ < (int)effectData.links.size()) {
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
	}
	ImGui::End();

	/// =========================
	/// 4. Debug Log
	/// =========================
	if (ImGui::Begin(kWinDebugLog)) {
		ImGui::Text("Debug Log:");
		ImGui::SameLine();
		if (ImGui::Button("Clear")) debugText_ = "";
		ImGui::SameLine();
		if (ImGui::Button("AddDummy")) debugText_ += "Dummy debug message\n";

		ImGui::BeginChild("DebugLogChild", ImVec2(0, 0), true);
		ImGui::TextUnformatted(debugText_.c_str());
		ImGui::SetScrollHereY(1.0f);
		ImGui::EndChild();
	}
	ImGui::End();
}

void EffectEditor::ImGuiMidWindow() {
	if (ImGui::Begin(kWinDrawingInfo)) {
		auto* pm = system_->GetEffectManager()->GetParticleManager();
		ImGui::Text("Emitter Num: %d", pm->GetEmitterCount());
		ImGui::Text("All Particle Num: %d", pm->GetAllParticleCount());
		ImGui::BeginChild("EmitterList", ImVec2(0, 0), true);
		ImGui::EndChild();
	}
	ImGui::End();

	if (ImGui::Begin(kWinParticleInfo)) {
	}
	ImGui::End();
}

void EffectEditor::ImGuiLoadWindow() {
	if (!showLoadWindow_) return;

	float lineHeight = ImGui::GetTextLineHeightWithSpacing();
	float padding = ImGui::GetStyle().WindowPadding.y * 2;
	float titleBar = ImGui::GetFrameHeight();
	float fixedHeight = lineHeight * 3 + padding + titleBar;

	/// メインウィンドウの真ん中に出す（ドック不可）
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));
	ImGui::SetNextWindowFocus();
	ImGui::Begin("Load Window", &showLoadWindow_, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

	InputTextString("Load File Path", loadFilePath_);

	if (ImGui::Button("Load")) {
		isLoad_ = true;
		showLoadWindow_ = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Close")) {
		showLoadWindow_ = false;
	}

	ImGui::End();
}

void EffectEditor::ImGuiSaveWindow() {
	if (!showSaveWindow_) return;

	float lineHeight = ImGui::GetTextLineHeightWithSpacing();
	float padding = ImGui::GetStyle().WindowPadding.y * 2;
	float titleBar = ImGui::GetFrameHeight();
	float fixedHeight = lineHeight * 3 + padding + titleBar;

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));
	ImGui::SetNextWindowFocus();
	ImGui::Begin("Save Window", &showSaveWindow_, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

	InputTextString("Save File Path", saveFilePath_);

	if (ImGui::Button("Save")) {
		isSave_ = true;
		showSaveWindow_ = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Close")) {
		saveLoadMode_ = SelectMode::None;
		showSaveWindow_ = false;
	}

	ImGui::End();
}

void EffectEditor::ImGuiDoubleCheckWindow() {
	if (!doubleCheck_) return;

	float lineHeight = ImGui::GetTextLineHeightWithSpacing();
	float padding = ImGui::GetStyle().WindowPadding.y * 2;
	float titleBar = ImGui::GetFrameHeight();
	float fixedHeight = lineHeight * 3 + padding + titleBar;

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));
	ImGui::SetNextWindowFocus();
	ImGui::Begin("Double Check Window", &doubleCheck_, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

	ImGui::Text("Are you sure you want to delete this item?");

	if (ImGui::Button("Delete")) {
		isDelete_ = true;
		doubleCheck_ = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Close")) {
		saveLoadMode_ = SelectMode::None;
		doubleCheck_ = false;
	}

	ImGui::End();
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