#include "SceneMapTest.h"

SceneMapTest::SceneMapTest(kEngine* system) {
	system_ = system;
	debugCamera_ = system_->CreateDebugCamera();
	debugCamera_.lock()->Move({ 0.0f, 0.0f, -30.0f });   // 少し離れて全体を見る

	phaseChangeTimer_ = new Timer();

	mapChipManager_ = std::make_unique<MapChipManager>(system_);
	/// 登録は無し：全部 SpawnDefault へ（直接タイル = ブロック、それ以外は名前だけ記録）
	mapChipManager_->SetDefault(this, &SceneMapTest::SpawnDefault);

	/// MapMaker の一覧（Generate 用）
	if (makerStore_.LoadProject(mmprojPath_)) {
		const auto handles = makerStore_.MakerHandles();
		if (!handles.empty()) makerHandle_ = handles[0];
	}
	for (const auto& l : makerStore_.Log()) log_ += l + "\n";
	makerStore_.Log().clear();
}

SceneMapTest::~SceneMapTest() {
	mapBlocks_.Clear();                // Object が先
	mapChipManager_.reset();
	system_->DestroyCamera(debugCamera_);
	delete phaseChangeTimer_;
}

/// ============================== 読み込み・生成 ============================== ///

bool SceneMapTest::LoadCsv() {
	mapBlocks_.Clear();
	mapChipManager_->RemoveLayer(kLayer_);
	if (!mapChipManager_->AddLayer(kLayer_, csvPath_)) {
		log_ += "CSV load failed: " + csvPath_ + "\n";
		return false;
	}
	log_ += "CSV loaded: " + csvPath_ + "\n";
	return true;
}

bool SceneMapTest::LoadGenerated() {
	const MapMakerAsset* maker = makerStore_.GetMaker(makerHandle_);
	if (!maker) { log_ += "No MapMaker selected\n"; return false; }

	/// 1. 空のレイヤー（MapMaker の大きさ・tutorial の MapData）
	mapBlocks_.Clear();
	if (!mapChipManager_->CreateLayer(kLayer_, mapDataPath_, maker->width, maker->height)) {
		log_ += "MapData load failed: " + mapDataPath_ + "\n";
		return false;
	}

	/// 2. 生成（空白の番号は MapData から）
	std::vector<std::string> errors;
	const TileMapData map = makerStore_.Generate(makerHandle_, mapChipManager_->GetBlankId(kLayer_), seed_, &errors);
	for (const auto& e : errors) log_ += e + "\n";

	/// 3. TileMapData → レイヤーの ID（Row[y][x]、y = 0 が一番上 = csv と同じ向き）
	for (int y = 0; y < map.Height(); ++y) {
		for (int x = 0; x < (int)map.Row[y].size(); ++x) {
			mapChipManager_->SetId(kLayer_, { x, y }, map.Row[y][x]);
		}
	}
	log_ += "Generated: " + maker->name + " (" + std::to_string(map.Width()) + "x" + std::to_string(map.Height()) + ")\n";
	return true;
}

void SceneMapTest::BuildBlocks() {
	unhandled_.clear();
	mapBlocks_.Init(mapChipManager_.get(), kLayer_);
	const MapReader::SpawnReport r = mapChipManager_->Generate(kLayer_);
	log_ += "Spawned: " + std::to_string(r.spawned) + "  Unhandled: " + std::to_string(r.unhandled) + "\n";
}

void SceneMapTest::SpawnDefault(const MapReader::SpawnInfo& s) {
	if (!mapBlocks_.OnSpawn(s)) {
		unhandled_.insert(s.unit ? s.unit->name : std::to_string(s.id));
	}
}

/// ============================== 更新・描画 ============================== ///

void SceneMapTest::Update() {
	/// 読み込みの予約を処理（ImGui のボタンからは予約だけ）
	if (request_ != Request::None) {
		const bool ok = (request_ == Request::Csv) ? LoadCsv() : LoadGenerated();
		if (ok) BuildBlocks();
		request_ = Request::None;
	}

	debugCamera_.lock()->MouseControlUpdate();
	system_->SetCamera(debugCamera_);
	mapBlocks_.Update();
}

void SceneMapTest::Draw() {
	const Vector3 camPos = debugCamera_.lock()->GetTransform().translate;
	mapBlocks_.Draw(camPos, drawRadius_);
#ifdef USE_IMGUI
	ImGuiPart();
#endif
}

#ifdef USE_IMGUI
void SceneMapTest::ImGuiPart() {
	ImGui::Begin("MapTest");

	/// ---- csv ----
	ImGui::SeparatorText("CSV");
	ImGui::InputText("csv", &csvPath_);
	if (ImGui::Button("Load CSV")) request_ = Request::Csv;

	/// ---- 生成 ----
	ImGui::SeparatorText("Generate");
	ImGui::InputText("mmproj", &mmprojPath_);
	ImGui::InputText("MapData", &mapDataPath_);
	if (ImGui::Button("Reload Project")) {
		makerStore_.LoadProject(mmprojPath_);
		const auto handles = makerStore_.MakerHandles();
		makerHandle_ = handles.empty() ? -1 : handles[0];
		for (const auto& l : makerStore_.Log()) log_ += l + "\n";
		makerStore_.Log().clear();
	}
	const MapMakerAsset* cur = makerStore_.GetMaker(makerHandle_);
	if (ImGui::BeginCombo("MapMaker", cur ? cur->name.c_str() : "(none)")) {
		for (int h : makerStore_.MakerHandles()) {
			if (ImGui::Selectable(makerStore_.GetMaker(h)->name.c_str(), h == makerHandle_)) makerHandle_ = h;
		}
		ImGui::EndCombo();
	}
	ImGui::InputInt("seed (-1 = MapMaker)", &seed_);
	if (ImGui::Button("Generate")) request_ = Request::Generate;

	/// ---- 表示 ----
	ImGui::SeparatorText("View");
	ImGui::SliderFloat("Draw Radius", &drawRadius_, 10.0f, 200.0f);
	const auto size = mapChipManager_->GetMapSize(kLayer_);
	ImGui::Text("Map: %d x %d", size.first, size.second);
	if (!unhandled_.empty()) {
		ImGui::Text("Not spawned:");
		for (const auto& n : unhandled_) { ImGui::SameLine(); ImGui::TextDisabled("%s", n.c_str()); }
	}

	/// ---- ログ ----
	ImGui::SeparatorText("Log");
	if (ImGui::SmallButton("Clear")) log_.clear();
	ImGui::BeginChild("log", ImVec2(0, 150), true);
	ImGui::TextUnformatted(log_.c_str());
	ImGui::EndChild();

	ImGui::End();
}
#endif