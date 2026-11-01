#include "EffectEditor.h"
#include <set>
#include <filesystem>
#include <algorithm>
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
	constexpr const char* kEffectFilter = "Effect (*.json)\0*.json\0";

	/// ファイルがあって、それが own（自分のファイル）ではない → true（上書き確認が必要）
	bool IsOtherFile(const std::string& target, const std::string& own) {
		std::error_code ec;
		if (!std::filesystem::exists(target, ec)) return false;
		if (!own.empty() && std::filesystem::equivalent(target, own, ec)) return false;
		return true;
	}

	constexpr int kPreviewMaxParticles = 1000;

	/// 見た目が無い Particle 用の既定の見た目（白い Plane のビルボード）
	ObjectData MakeDefaultParticleObject() {
		ObjectData obj;
		obj.isBillboard_ = true;
		obj.modelHandle_ = config::default_Plane_MeshBufferHandle_;

		ObjectPart part;
		part.name = "Default";
		part.materialConfig = std::make_shared<MaterialConfig>();
		InitMaterialConfig(part.materialConfig.get());
		part.materialConfig->MakePSOParticle();
		part.materialConfig->enableLighting = false;
		part.materialConfig->textureHandle = config::default_White5x5_TextureHandle_;
		obj.objectParts_.push_back(part);
		return obj;
	}
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
	//HitSpark hitSpark;
	//hitSpark.startPosition = { 0.0f, 0.0f, 0.0f };
	//hitSpark.objectList[0].objectParts_[0].materialConfig->textureHandle = TH_clicleTextureHandle_;
	//TH_particleHandle_ = system_->GetEffectManager()->GetParticleManager()->CreateEmitter(hitSpark, 0);
	//
	//HitImpact hitImpact;
	//hitImpact.startPosition = { 0.0f, 0.0f, 0.0f };
	//hitImpact.objectList[0].objectParts_[0].materialConfig->textureHandle = TH_effectTextureHandle_;
	//TH_particleHandle2_ = system_->GetEffectManager()->GetParticleManager()->CreateEmitter(hitImpact, 1);
	//
	//HitSpackImpactLink linkData;
	//system_->GetEffectManager()->GetParticleManager()->LinkEmitterToEmitter(linkData);

	/// ========== デバッグ用 ============///
	//Effect_HitSpark debugEffect;
	//debugEffect.prototypes[0].objectList[0].objectParts_[0].materialConfig->textureHandle = TH_clicleTextureHandle_;   // HitSpark
	//debugEffect.prototypes[1].objectList[0].objectParts_[0].materialConfig->textureHandle = TH_effectTextureHandle_;   // HitImpact
	//effectDataList_.push_back(debugEffect);

}

EffectEditor::~EffectEditor() {}

void EffectEditor::Update() {

	SaveLoadPart();
	DeletePart();

	centerAnchor_->Update();
	if (!isPause_) {
		if (isShootNonStop_) {
			if (shootTime.foreverUp()) {
				ShootPreview();
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
}

void EffectEditor::SavePart() {
	/// 上書き確認の返事待ちなら止まる
	if (pendingSave_.active) return;

	while (!saveQueue_.empty()) {
		SaveRequest request = saveQueue_.front();
		saveQueue_.erase(saveQueue_.begin());

		BeginSave(request);
		if (pendingSave_.active) break;   // 確認が必要になった → 返事が来るまで次へ行かない
	}
}

void EffectEditor::LoadEffectFromDialog() {
	namespace fs = std::filesystem;

	if (pendingSave_.active || !saveQueue_.empty()) {
		debugText_ += "Error: Cannot load while saving.\n";
		return;
	}

	const std::string path = FileManager::OpenFileDialog(kEffectFilter, "Load Effect");
	if (path.empty()) return;   // キャンセル

	/// もう開いている Effect → 2つ目は作らずにそれを選ぶ
	for (int i = 0; i < (int)effectDataList_.size(); i++) {
		std::error_code ec;
		if (!effectDataList_[i].savePath.empty() && fs::equivalent(effectDataList_[i].savePath, path, ec)) {
			currentEffectIndex_ = i;
			currentParticleIndex_ = -1;
			currentLinkIndex_ = -1;
			selectMode_ = SelectMode::Effect;
			debugText_ += "Already opened: " + effectDataList_[i].name + "\n";
			return;
		}
	}

	EffectData effect;
	std::string log;
	const bool ok = effectAdapter_.LoadEffect(path, effect, &log);
	debugText_ += log;
	if (!ok) return;

	effectDataList_.push_back(std::move(effect));
	currentEffectIndex_ = (int)effectDataList_.size() - 1;
	currentParticleIndex_ = -1;
	currentLinkIndex_ = -1;
	selectMode_ = SelectMode::Effect;
	debugText_ += "Loaded Effect: " + path + "\n";
}

void EffectEditor::ImportUnitFromDialog(SelectMode target) {
	namespace fs = std::filesystem;

	if (!isEffectSelected_) {
		debugText_ += "Error: No effect selected for import.\n";
		return;
	}
	if (pendingSave_.active || !saveQueue_.empty()) {
		debugText_ += "Error: Cannot import while saving.\n";
		return;
	}

	const bool isParticle = (target == SelectMode::Particle);
	const std::string path = FileManager::OpenFileDialog(kEffectFilter, isParticle ? "Import Particle" : "Import EmitterLink");
	if (path.empty()) return;

	EffectData& effect = effectDataList_[currentEffectIndex_];

	/// 同じファイルがもうこの Effect にある → 追加しない
	auto samePath = [&](const std::string& p) {
		std::error_code ec;
		return !p.empty() && fs::equivalent(p, path, ec);
		};
	if (isParticle) {
		for (const auto& p : effect.prototypes) if (samePath(p.savePath)) { debugText_ += "Already in effect: " + p.name + "\n"; return; }
	} else {
		for (const auto& l : effect.links) if (samePath(l.savePath)) { debugText_ += "Already in effect: " + l.name + "\n"; return; }
	}

	std::string log;
	bool ok = false;
	if (isParticle) {
		ParticlePrototype proto;
		ok = effectAdapter_.LoadParticle(path, proto, &log);
		if (ok) {
			effect.prototypes.push_back(std::move(proto));
			currentParticleIndex_ = (int)effect.prototypes.size() - 1;
			currentLinkIndex_ = -1;
		}
	} else {
		EmitterLink link;
		ok = effectAdapter_.LoadEmitterLink(path, link, &log);
		if (ok) {
			effect.links.push_back(std::move(link));
			currentLinkIndex_ = (int)effect.links.size() - 1;
			currentParticleIndex_ = -1;
		}
	}
	debugText_ += log;
	if (!ok) return;

	selectMode_ = target;
	effect.isSaved = false;   // Effect ファイルに参照を足す必要がある
	debugText_ += std::string("Imported ") + (isParticle ? "Particle: " : "Link: ") + path + "\n";
}

void EffectEditor::RequestSave(int effectIndex, bool forceDialog) {
	SaveRequest request;
	request.target = SelectMode::Effect;
	request.effectIndex = effectIndex;
	request.forceDialog = forceDialog;
	saveQueue_.push_back(request);
}

void EffectEditor::RequestSaveUnit(SelectMode target, int effectIndex, int itemIndex, bool forceDialog) {
	SaveRequest request;
	request.target = target;
	request.effectIndex = effectIndex;
	request.itemIndex = itemIndex;
	request.forceDialog = forceDialog;
	saveQueue_.push_back(request);
}

void EffectEditor::RequestSaveAll() {
	for (int i = 0; i < (int)effectDataList_.size(); i++) {
		if (!effectDataList_[i].isSaved) RequestSave(i, false);
	}
}

void EffectEditor::BeginSave(const SaveRequest& request) {
	switch (request.target) {
	case SelectMode::Effect:

		BeginSaveEffect(request);
		break;

	case SelectMode::Particle:
	case SelectMode::Linker:

		BeginSaveUnit(request);
		break;

	default:
		break;
	}
}

void EffectEditor::BeginSaveEffect(const SaveRequest& request) {
	if (request.effectIndex < 0 || request.effectIndex >= (int)effectDataList_.size()) {
		debugText_ += "Error: Invalid effect index for saving.\n";
		return;
	}
	EffectData& effect = effectDataList_[request.effectIndex];

	/// --- 保存先 ---
	std::string path = effect.savePath;
	bool detach = false;
	if (request.forceDialog || path.empty()) {
		path = AskEffectPath(effect);
		if (path.empty()) return;          // キャンセル
		detach = true;                     // 新しい場所 → 元のフォルダの中身も新しい場所へ
	}

	/// --- 同名チェック（Linkが名前で区別できなくなるので警告。保存は止めない） ---
	std::set<std::string> names;
	for (const auto& p : effect.prototypes) {
		if (!names.insert(p.name).second) {
			debugText_ += "Warning: duplicate particle name \"" + p.name + "\" in " + effect.name + "\n";
		}
	}

	/// --- 計画（detach の場合はコピーで計画して、確認前に本物を変えない） ---
	EffectData planned = effect;
	if (detach) DetachEffectFolder(planned);
	EffectSavePlan plan = effectAdapter_.PlanEffect(planned, path);

	/// --- 衝突チェック ---
	std::vector<std::string> conflicts = FindConflicts(planned, plan);
	if (conflicts.empty()) {
		ExecuteSave(request.effectIndex, plan, detach);
		return;
	}

	pendingSave_ = {};
	pendingSave_.active = true;
	pendingSave_.request = request;
	pendingSave_.detach = detach;
	pendingSave_.plan = plan;
	pendingSave_.conflicts = conflicts;
}

void EffectEditor::BeginSaveUnit(const SaveRequest& request) {
	namespace fs = std::filesystem;

	if (request.effectIndex < 0 || request.effectIndex >= (int)effectDataList_.size()) {
		debugText_ += "Error: Invalid effect index for saving.\n";
		return;
	}
	EffectData& effect = effectDataList_[request.effectIndex];

	const bool isParticle = (request.target == SelectMode::Particle);
	const int count = isParticle ? (int)effect.prototypes.size() : (int)effect.links.size();
	if (request.itemIndex < 0 || request.itemIndex >= count) {
		debugText_ += "Error: Invalid item index for saving.\n";
		return;
	}

	const std::string& name = isParticle ? effect.prototypes[request.itemIndex].name : effect.links[request.itemIndex].name;
	const std::string& own = isParticle ? effect.prototypes[request.itemIndex].savePath : effect.links[request.itemIndex].savePath;
	const char* folder = isParticle ? "Particle" : "EmitterLink";
	const std::string fileName = FileManager::ToSafeFileName(name) + ".json";

	/// --- 保存先 ---
	std::string path = own;
	if (request.forceDialog || path.empty()) {
		if (!request.forceDialog && !effect.savePath.empty()) {
			/// まだ保存してない + Effect は保存済み → Effectフォルダの下（既定の場所）
			path = (fs::path(effect.savePath).parent_path() / folder / fileName).string();
		} else {
			/// Save As / Effect もまだ → 好きな場所を選ぶ（共用フォルダなど）
			path = FileManager::SaveFileDialog(kEffectFilter, "json", fileName,
				isParticle ? "Save Particle" : "Save EmitterLink");
			if (path.empty()) return;   // キャンセル
		}
	}

	/// --- 衝突チェック ---
	if (!IsOtherFile(path, own)) {
		ExecuteSaveUnit(request, path);
		return;
	}

	pendingSave_ = {};
	pendingSave_.active = true;
	pendingSave_.request = request;
	pendingSave_.unitPath = path;
	pendingSave_.conflicts = { path };
}

bool EffectEditor::ExecuteSave(int effectIndex, const EffectSavePlan& plan, bool detach) {
	if (effectIndex < 0 || effectIndex >= (int)effectDataList_.size()) {
		debugText_ += "Error: Invalid effect index for saving.\n";
		return false;
	}
	EffectData& effect = effectDataList_[effectIndex];

	if (detach) DetachEffectFolder(effect);

	std::string error;
	if (!effectAdapter_.WriteEffect(effect, plan, useRelativePath_, &error)) {
		debugText_ += "Save Error: " + error + "\n";
		return false;
	}

	effect.savePath = plan.effectPath;
	effect.isSaved = true;
	for (auto& p : effect.prototypes) p.isSaved = true;
	for (auto& l : effect.links) l.isSaved = true;

	debugText_ += "Saved Effect: " + plan.effectPath + "\n";
	return true;
}

bool EffectEditor::ExecuteSaveUnit(const SaveRequest& request, const std::string& path) {
	if (request.effectIndex < 0 || request.effectIndex >= (int)effectDataList_.size()) {
		debugText_ += "Error: Invalid effect index for saving.\n";
		return false;
	}
	EffectData& effect = effectDataList_[request.effectIndex];

	const bool isParticle = (request.target == SelectMode::Particle);
	const int count = isParticle ? (int)effect.prototypes.size() : (int)effect.links.size();
	if (request.itemIndex < 0 || request.itemIndex >= count) {
		debugText_ += "Error: Invalid item index for saving.\n";
		return false;
	}

	std::string error;
	std::string* savePath = nullptr;
	bool* isSaved = nullptr;
	bool ok = false;

	if (isParticle) {
		auto& p = effect.prototypes[request.itemIndex];
		ok = effectAdapter_.WriteParticle(p, path, useRelativePath_, &error);
		savePath = &p.savePath;
		isSaved = &p.isSaved;
	} else {
		auto& l = effect.links[request.itemIndex];
		ok = effectAdapter_.WriteEmitterLink(l, path, &error);
		savePath = &l.savePath;
		isSaved = &l.isSaved;
	}

	if (!ok) {
		debugText_ += "Save Error: " + error + "\n";
		return false;
	}

	/// 場所が変わった → Effectファイルの中の参照も直す必要がある
	std::error_code ec;
	const bool moved = savePath->empty() || !std::filesystem::equivalent(*savePath, path, ec);

	*savePath = path;
	*isSaved = true;
	if (moved) {
		effect.isSaved = false;
		debugText_ += "Note: location changed. Save the effect to update its reference.\n";
	}

	debugText_ += std::string("Saved ") + (isParticle ? "Particle: " : "Link: ") + path + "\n";
	return true;
}

std::vector<std::string> EffectEditor::FindConflicts(const EffectData& effect, const EffectSavePlan& plan) const {
	std::vector<std::string> result;
	if (IsOtherFile(plan.effectPath, effect.savePath)) result.push_back(plan.effectPath);
	for (size_t i = 0; i < effect.prototypes.size() && i < plan.particlePaths.size(); i++) {
		if (IsOtherFile(plan.particlePaths[i], effect.prototypes[i].savePath)) result.push_back(plan.particlePaths[i]);
	}
	for (size_t i = 0; i < effect.links.size() && i < plan.linkPaths.size(); i++) {
		if (IsOtherFile(plan.linkPaths[i], effect.links[i].savePath)) result.push_back(plan.linkPaths[i]);
	}
	return result;
}

std::string EffectEditor::AskEffectPath(const EffectData& effect) {
	namespace fs = std::filesystem;
	const std::string safeName = FileManager::ToSafeFileName(effect.name);

	const std::string picked = FileManager::SaveFileDialog(kEffectFilter, "json", safeName + ".json",
		"Save Effect (choose output folder)");
	if (picked.empty()) return "";

	/// 選んだ場所がもう <Effect名> フォルダならそのまま、違うなら <Effect名> フォルダを作る
	/// ファイル名は入力に関係なく <Effect名>.json に揃える
	fs::path dir = fs::path(picked).parent_path();
	if (dir.filename().string() != safeName) dir /= safeName;
	return (dir / (safeName + ".json")).string();
}

void EffectEditor::DetachEffectFolder(EffectData& effect) {
	namespace fs = std::filesystem;
	if (!effect.savePath.empty()) {
		const std::string dir = fs::path(effect.savePath).parent_path().generic_string() + "/";
		auto isInside = [&](const std::string& p) {
			return !p.empty() && fs::path(p).generic_string().rfind(dir, 0) == 0;
			};
		for (auto& p : effect.prototypes) if (isInside(p.savePath)) p.savePath.clear();
		for (auto& l : effect.links)      if (isInside(l.savePath)) l.savePath.clear();
	}
	effect.savePath.clear();
}

void EffectEditor::RenameParticleInLinks(EffectData& effect, const std::string& oldName, const std::string& newName) {
	for (auto& link : effect.links) {
		bool changed = false;
		if (link.sourceName == oldName) { link.sourceName = newName; changed = true; }
		if (link.targetName == oldName) { link.targetName = newName; changed = true; }
		if (changed) link.isSaved = false;   // Link のファイルも保存し直す必要がある
	}
}

void EffectEditor::RefreshPreview() {
	if (!isEffectSelected_) return;
	if (!HasValidEffect()) return;
	const EffectData& effect = effectDataList_[currentEffectIndex_];

	const bool needRebuild =
		isPreviewDirty_ ||
		previewEffectIndex_ != currentEffectIndex_ ||
		previewParticleCount_ != effect.prototypes.size() ||
		previewLinkCount_ != effect.links.size();
	if (!needRebuild) return;

	auto* pm = system_->GetEffectManager()->GetParticleManager();

	/// Link は全部作り直す（前の Effect の Link が残らないように）
	pm->ClearEmitterLinks();

	/// Emitter：同じ名前があれば中身だけ更新される（飛んでいる粒子は最後まで残る）
	std::vector<std::pair<std::string, int>> emitters;   // 名前, id
	for (const auto& src : effect.prototypes) {
		ParticlePrototype proto = src;
		if (proto.objectList.empty()) proto.objectList.push_back(MakeDefaultParticleObject());   // 見た目が無いと描画で範囲外
		emitters.push_back({ proto.name, pm->CreateEmitter(proto, kPreviewMaxParticles) });
	}

	/// 前のプレビューにあって今回は無い線（名前を変えた・消した・別の Effect のもの）→ 引退
	std::vector<int> newIds;
	for (const auto& [name, id] : emitters) newIds.push_back(id);
	for (int oldId : previewEmitterIds_) {
		if (std::find(newIds.begin(), newIds.end(), oldId) == newIds.end()) {
			pm->RetireEmitter(oldId);
		}
	}
	previewEmitterIds_ = newIds;

	for (const auto& src : effect.links) {
		EmitterLink link = src;                 // LinkEmitterToEmitter が非 const 参照なのでコピー
		pm->LinkEmitterToEmitter(link);
	}

	/// isRoot の Particle = Shoot で直接撃つもの
	previewRootIds_.clear();
	for (size_t i = 0; i < effect.prototypes.size(); i++) {
		if (effect.prototypes[i].isRoot) previewRootIds_.push_back(emitters[i].second);
	}

	previewEffectIndex_ = currentEffectIndex_;
	previewParticleCount_ = effect.prototypes.size();
	previewLinkCount_ = effect.links.size();
	isPreviewDirty_ = false;
}

void EffectEditor::ShootPreview() {
	if (!isEffectSelected_) {
		debugText_ += "Error: No effect selected for shooting.\n";
		return;
	}
	if (!HasValidEffect()) {
		debugText_ += "Error: No effect selected for shooting.\n";
		isShootNonStop_ = false;   // 連発中に消された → 連発も止める（Log が毎秒出続けないように）
		return;
	}
	RefreshPreview();
	if (previewRootIds_.empty()) {
		debugText_ += "Warning: no Root particle in this effect.\n";
		return;
	}

	auto* pm = system_->GetEffectManager()->GetParticleManager();
	for (int id : previewRootIds_) pm->ShootEmitter(id, 1);
}


void EffectEditor::DeletePart() {
	if (!isDelete_) return;
	isDelete_ = false;

	if (pendingSave_.active || !saveQueue_.empty()) {
		debugText_ += "Error: Cannot delete while saving.\n";
		return;
	}

	SelectMode mode = saveLoadMode_;
	saveLoadMode_ = SelectMode::None;

	if (currentEffectIndex_ < 0 || currentEffectIndex_ >= (int)effectDataList_.size()) {
		debugText_ += "Error: Invalid currentEffectIndex_ for delete.\n";
		return;
	}
	auto& effect = effectDataList_[currentEffectIndex_];

	isPreviewDirty_ = true;
	switch (mode) {
	case SelectMode::Effect:
		effectDataList_.erase(effectDataList_.begin() + currentEffectIndex_);
		/// 消したら選択をリセット
		currentEffectIndex_ = -1;
		currentParticleIndex_ = -1;
		currentLinkIndex_ = -1;
		selectMode_ = SelectMode::None;
		isEffectSelected_ = false;
		break;

	case SelectMode::Particle:
		if (currentParticleIndex_ >= 0 && currentParticleIndex_ < (int)effect.prototypes.size()) {
			effect.prototypes.erase(effect.prototypes.begin() + currentParticleIndex_);
			effect.isSaved = false;
			currentParticleIndex_ = -1;
		} else {
			debugText_ += "Error: Invalid currentParticleIndex_ for delete.\n";
		}
		break;

	case SelectMode::Linker:
		if (currentLinkIndex_ >= 0 && currentLinkIndex_ < (int)effect.links.size()) {
			effect.links.erase(effect.links.begin() + currentLinkIndex_);
			effect.isSaved = false;
			currentLinkIndex_ = -1;
		} else {
			debugText_ += "Error: Invalid currentLinkIndex_ for delete.\n";
		}
		break;

	default:
		break;
	}
}

#ifdef USE_IMGUI
void EffectEditor::ImGuiPart() {

	/// ============== ショートカット =============== ///
	if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S) && isEffectSelected_) {
		RequestSave(currentEffectIndex_, false);
	}

	/// ============== メニューバー =============== ///
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {};
			if (ImGui::MenuItem("Load Effect...")) LoadEffectFromDialog();
			if (ImGui::MenuItem("Import Particle...", nullptr, false, isEffectSelected_)) ImportUnitFromDialog(SelectMode::Particle);
			if (ImGui::MenuItem("Import EmitterLink...", nullptr, false, isEffectSelected_)) ImportUnitFromDialog(SelectMode::Linker);
			ImGui::Separator();
			if (ImGui::MenuItem("Save Effect", "Ctrl+S", false, isEffectSelected_)) {
				RequestSave(currentEffectIndex_, false);
			}
			if (ImGui::MenuItem("Save Effect As...", nullptr, false, isEffectSelected_)) {
				RequestSave(currentEffectIndex_, true);
			}
			if (ImGui::MenuItem("Save All")) RequestSaveAll();
			ImGui::Separator();
			ImGui::MenuItem("Relative Paths", nullptr, &useRelativePath_);
			ImGui::SetItemTooltip("ON : 保存するファイルから見た相対パス（フォルダごと移動しても読める）\n"
				"OFF: 絶対パス（別ドライブの素材など）");
			if (ImGui::MenuItem("Exit")) {
				outcome_ = SceneOutcome::EXIT;
				isSceneEnd_ = true;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Reset Layout")) isResetLayout_ = true;
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	/// ============ ドックスペース（各ウィンドウより先に） ============= ///
	ImGuiDockSpace();

	ImGuiLeftMenuBar();
	ImGuiRightMenuBar();
	ImGuiMidWindow();

	/// ============ Popup Window ============= ///
	ImGuiDoubleCheckWindow();
	ImGuiOverwriteWindow();
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
				DetachEffectFolder(newEffect);   // 元のフォルダに上書きしないように
				newEffect.isSaved = false;
				effectDataList_.push_back(newEffect);
			} else {
				debugText_ += "Error: No effect selected for doubling.\n";
			}
		}
		ImGui::SameLine();

		/// Save
		if (ImGui::Button("Save", ImVec2(buttonW, 0))) {
			if (isEffectSelected_) RequestSave(currentEffectIndex_, false);
			else debugText_ += "Error: No effect selected for saving.\n";
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
			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::MenuItem("Save")) RequestSave(i, false);
				if (ImGui::MenuItem("Save As...")) RequestSave(i, true);
				ImGui::EndPopup();
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
				newParticle.objectList.push_back(MakeDefaultParticleObject());
				effectDataList_[currentEffectIndex_].prototypes.push_back(newParticle);
				effectDataList_[currentEffectIndex_].isSaved = false;
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
					newPrototype.savePath.clear();
					newPrototype.isSaved = false;
					prototypes.push_back(newPrototype);
					effectDataList_[currentEffectIndex_].isSaved = false;
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
				auto& prototypes = effectDataList_[currentEffectIndex_].prototypes;
				if (currentParticleIndex_ >= 0 && currentParticleIndex_ < (int)prototypes.size()) {
					RequestSaveUnit(SelectMode::Particle, currentEffectIndex_, currentParticleIndex_, false);
				} else {
					debugText_ += "Error: Invalid currentParticleIndex_ for saving.\n";
				}
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
				std::string label = std::string(prototype.isRoot ? "> " : "  ") + prototype.name + (prototype.isSaved ? "" : " *");
				ImGui::PushID(i);
				if (ImGui::Selectable(label.c_str(), currentParticleIndex_ == i)) {
					currentParticleIndex_ = i;
					currentLinkIndex_ = -1;
					selectMode_ = SelectMode::Particle;
				}
				/// 右クリックメニュー
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Save")) RequestSaveUnit(SelectMode::Particle, currentEffectIndex_, i, false);
					if (ImGui::MenuItem("Save As...")) RequestSaveUnit(SelectMode::Particle, currentEffectIndex_, i, true);
					ImGui::EndPopup();
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
				effectDataList_[currentEffectIndex_].isSaved = false;
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
					newLink.savePath.clear();
					newLink.isSaved = false;
					links.push_back(newLink);
					effectDataList_[currentEffectIndex_].isSaved = false;
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
				auto& links = effectDataList_[currentEffectIndex_].links;
				if (currentLinkIndex_ >= 0 && currentLinkIndex_ < (int)links.size()) {
					RequestSaveUnit(SelectMode::Linker, currentEffectIndex_, currentLinkIndex_, false);
				} else {
					debugText_ += "Error: Invalid currentLinkIndex_ for saving.\n";
				}
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
				/// 右クリックメニュー
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Save")) RequestSaveUnit(SelectMode::Linker, currentEffectIndex_, i, false);
					if (ImGui::MenuItem("Save As...")) RequestSaveUnit(SelectMode::Linker, currentEffectIndex_, i, true);
					ImGui::EndPopup();
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
			ShootPreview();
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
		bool linkLog = pm->GetLinkLog();
		ImGui::Text("Link Log:");
		ImGui::SameLine();
		if (ImGui::Checkbox("##LinkLog", &linkLog)) pm->SetLinkLog(linkLog);
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
				if (ImGuiEffectShowing(effectData)) { 
					effectData.isSaved = false;
					isPreviewDirty_ = true; 
				}
				break;
			case SelectMode::Particle:
				if (currentParticleIndex_ >= 0 && currentParticleIndex_ < (int)effectData.prototypes.size()) {
					auto& proto = effectData.prototypes[currentParticleIndex_];
					const std::string oldName = proto.name;      // 変更前の名前を覚えておく

					if (ImGuiParticleShowing(proto)) {
						proto.isSaved = false;
						effectData.isSaved = false;
						isPreviewDirty_ = true;

						/// 名前が変わった → この Effect の Link も新しい名前に追従させる
						if (proto.name != oldName) {
							RenameParticleInLinks(effectData, oldName, proto.name);
						}
					}
				}
				break;
			case SelectMode::Linker:
				if (currentLinkIndex_ >= 0 && currentLinkIndex_ < (int)effectData.links.size()) {
					auto& link = effectData.links[currentLinkIndex_];
					if (ImGuiLinkerShowing(link)) {
						link.isSaved = false;
						effectData.isSaved = false;
						isPreviewDirty_ = true;
					}
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

void EffectEditor::ImGuiOverwriteWindow() {
	if (!pendingSave_.active) return;

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(520, 0), ImGuiCond_Appearing);
	ImGui::SetNextWindowFocus();
	ImGui::Begin("Overwrite?", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse);


	const auto& req = pendingSave_.request;
	const bool validIndex = req.effectIndex >= 0 && req.effectIndex < (int)effectDataList_.size();
	const char* kindName = (req.target == SelectMode::Effect) ? "Effect"
		: (req.target == SelectMode::Particle) ? "Particle" : "EmitterLink";
	ImGui::Text("%s of: %s", kindName, validIndex ? effectDataList_[req.effectIndex].name.c_str() : "(removed)");
	ImGui::Text("These files already exist and will be overwritten:");

	ImGui::BeginChild("ConflictList", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 6), true);
	for (const auto& path : pendingSave_.conflicts) {
		ImGui::TextUnformatted(path.c_str());
	}
	ImGui::EndChild();

	if (ImGui::Button("Overwrite")) {
		if (req.target == SelectMode::Effect) {
			ExecuteSave(req.effectIndex, pendingSave_.plan, pendingSave_.detach);
		} else {
			ExecuteSaveUnit(req, pendingSave_.unitPath);
		}
		pendingSave_ = {};
	}
	ImGui::SameLine();
	if (ImGui::Button("Skip")) {
		debugText_ += "Save skipped.\n";
		pendingSave_ = {};                  // この Effect だけ飛ばす（Save All なら次へ）
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel All")) {
		debugText_ += "Save cancelled.\n";
		pendingSave_ = {};
		saveQueue_.clear();                 // 残りの予約も全部やめる
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

bool EffectEditor::ImGuiEffectShowing(EffectData& effect) {
	bool changed = false;
	changed |= InputTextString("Effect Name", effect.name);
	ImGui::Text("Particles: %d", (int)effect.prototypes.size());
	ImGui::Text("Links: %d", (int)effect.links.size());
	ImGui::Text("Path: %s", effect.savePath.empty() ? "(not saved)" : effect.savePath.c_str());
	return changed;
}

bool EffectEditor::ImGuiParticleShowing(ParticlePrototype& particle) {
	bool changed = false;

	changed |= InputTextString("Particle Name", particle.name);
	ImGui::TextWrapped("Path: %s", particle.savePath.empty() ? "(not saved)" : particle.savePath.c_str());
	changed |= ImGui::Checkbox("Root (Shoot on Play)", &particle.isRoot);
	ImGui::SetItemTooltip("ON : Shoot / Play で直接撃つ\nOFF: Link からだけ撃たれる");

	/// ヘッダーごとに PushID して、同じラベル（Start / isConstant など）のIDが被らないようにする
	ImGui::PushID("Lifetime");
	if (ImGui::CollapsingHeader("Lifetime")) {
		changed |= InputFloat("lifetime", particle.lifetime);
		changed |= InputFloat("lifetimeRandomness", particle.lifetimeRandomness);
	}
	ImGui::PopID();

	ImGui::PushID("Position");
	if (ImGui::CollapsingHeader("Position")) {
		changed |= InputVector3("Start", particle.startPosition);
		changed |= InputVector3("StartRandom", particle.startPositionRandom);
	}
	ImGui::PopID();

	ImGui::PushID("Velocity");
	if (ImGui::CollapsingHeader("Velocity")) {
		changed |= InputVector3("StartVelocity", particle.startVelocity);
		changed |= InputVector3("StartRandom", particle.startVelocityRandom);
		changed |= InputVector3("StartSpeedRandom", particle.startSpeedRandom);
	}
	ImGui::PopID();

	ImGui::PushID("Scale");
	if (ImGui::CollapsingHeader("Scale")) {
		changed |= InputVector3("Start", particle.startScale);
		changed |= InputVector3("End", particle.endScale);
		changed |= InputVector3("StartRandom", particle.startScaleRandom);
		changed |= InputVector3("EndRandom", particle.endScaleRandom);
		changed |= ImGui::Checkbox("isConstant", &particle.isConstantScale);
	}
	ImGui::PopID();

	ImGui::PushID("Rotation");
	if (ImGui::CollapsingHeader("Rotation")) {
		changed |= InputVector3("Start", particle.startRotation);
		changed |= InputVector3("End", particle.endRotation);
		changed |= InputVector3("StartRandom", particle.startRotationRandom);
		changed |= InputVector3("EndRandom", particle.endRotationRandom);
		changed |= ImGui::Checkbox("isConstant", &particle.isConstantRotation);
	}
	ImGui::PopID();

	ImGui::PushID("Color");
	if (ImGui::CollapsingHeader("Color")) {
		changed |= InputVector4("Start", particle.startColor);
		changed |= InputVector4("End", particle.endColor);
		changed |= InputVector4("StartRandom", particle.startColorRandom);
		changed |= InputVector4("EndRandom", particle.endColorRandom);
		changed |= ImGui::Checkbox("isConstant", &particle.isConstantColor);
	}
	ImGui::PopID();

	ImGui::PushID("Movement");
	if (ImGui::CollapsingHeader("Movement")) {
		changed |= InputVector3("Gravity", particle.gravity);
		changed |= InputFloat("damping", particle.damping);
		changed |= InputFloat("dampingRandom", particle.dampingRandom);
	}
	ImGui::PopID();

	ImGui::PushID("Emitting");
	if (ImGui::CollapsingHeader("EmittingSettings")) {
		changed |= InputFloat("Rate", particle.emitRate);
		changed |= InputFloat("IntervalRandom", particle.emitIntervalRandom);
		changed |= InputInt("BurstCount", particle.burstCount);
		changed |= InputFloat("EmitNumRandom", particle.emitNumRandom);
	}
	ImGui::PopID();

	return changed;
}

bool EffectEditor::ImGuiLinkerShowing(EmitterLink& linker) {
	bool changed = false;

	changed |= InputTextString("Linker Name", linker.name);
	ImGui::TextWrapped("Path: %s", linker.savePath.empty() ? "(not saved)" : linker.savePath.c_str());
	changed |= InputTextString("Source Name", linker.sourceName);
	changed |= InputTextString("Target Name", linker.targetName);
	changed |= InputInt("Emit Count", linker.emitCount);
	changed |= InputFloat("Delay Time", linker.delayTime);

	/// データにはあったがUIが無かった3つ
	changed |= ImGui::Checkbox("Follow Position", &linker.followPosition);
	changed |= ImGui::Checkbox("Follow Rotation", &linker.followRotation);
	changed |= ImGui::Checkbox("Follow Scale", &linker.followScale);

	const char* linkModeItems[] = { "PerBurst", "PerParticle" };
	changed |= EnumCombo("LinkMode", linker.linkMode, linkModeItems, IM_ARRAYSIZE(linkModeItems));

	const char* linkFollowItems[] = { "Particle", "Emitter" };
	changed |= EnumCombo("LinkFollow", linker.linkFollow, linkFollowItems, IM_ARRAYSIZE(linkFollowItems));

	const char* emitterTimingItems[] = { "SourceEmit", "TimeUp", "SourceEnd" };
	changed |= EnumCombo("Emitter Timing", linker.emitterTiming, emitterTimingItems, IM_ARRAYSIZE(emitterTimingItems));

	return changed;
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

bool EffectEditor::InputInt(const char* label, int& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	return ImGui::InputInt(newLabel.c_str(), &value);
}

bool EffectEditor::InputFloat(const char* label, float& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	return ImGui::InputFloat(newLabel.c_str(), &value);
}

bool EffectEditor::InputVector3(const char* label, Vector3& vec) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	return ImGui::InputFloat3(newLabel.c_str(), &vec.x);
}
bool EffectEditor::InputVector4(const char* label, Vector4& vec) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	return ImGui::InputFloat4(newLabel.c_str(), &vec.x);
}
#endif

/// ImDrawList* draw = ImGui::GetWindowDrawList();
/// ImVec2 p = ImGui::GetCursorScreenPos();
/// draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 100, p.y + 50), IM_COL32(0, 255, 0, 255));