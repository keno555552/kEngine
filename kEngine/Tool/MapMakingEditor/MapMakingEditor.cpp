#include <algorithm>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <set>
#include "Tool/JsonHelper/JsonHelper.h"
#include "Tool/FileManager/FileManager.h"
#include "MapMakingEditor.h"
#include "DebugDraw.h"
#include "ImguiManager.h"
#include "ImGuiWidgets.h"
#include "config.h"

#include "Renderer/Resource/ResourceManager.h"
#include "Renderer/Mesh/ModelGroup.h"
#include "Renderer/Mesh/VertexIndex.h"

#ifdef USE_IMGUI
namespace {
	constexpr int kDefaultMapW = 40;
	constexpr int kDefaultMapH = 20;
	constexpr int kMaxPreviewInstances = 8000;          // config の 3D インスタンス上限 10000 より少し下

	constexpr const char* kJsonFilter = "JSON (*.json)\0*.json\0";

	using CellList = std::vector<std::pair<int, int>>;

	/// 四角：2マスを角とする範囲。filled = false なら外周だけ
	CellList RectCells(int x0, int y0, int x1, int y1, bool filled) {
		const int minX = (std::min)(x0, x1), maxX = (std::max)(x0, x1);
		const int minY = (std::min)(y0, y1), maxY = (std::max)(y0, y1);
		CellList out;
		for (int y = minY; y <= maxY; ++y) {
			for (int x = minX; x <= maxX; ++x) {
				if (filled || x == minX || x == maxX || y == minY || y == maxY) out.push_back({ x, y });
			}
		}
		return out;
	}

	/// 楕円：範囲に内接。filled = false なら「中にあって、上下左右のどれかが外」のマスだけ
	CellList EllipseCells(int x0, int y0, int x1, int y1, bool filled) {
		const int minX = (std::min)(x0, x1), maxX = (std::max)(x0, x1);
		const int minY = (std::min)(y0, y1), maxY = (std::max)(y0, y1);
		const float rx = (maxX - minX + 1) * 0.5f;          // 半径（マス単位）
		const float ry = (maxY - minY + 1) * 0.5f;
		const float cxf = minX + rx, cyf = minY + ry;       // 中心（マスの角の座標）

		auto inside = [&](int x, int y) {
			const float dx = (x + 0.5f - cxf) / rx;          // マスの中心で判定
			const float dy = (y + 0.5f - cyf) / ry;
			return dx * dx + dy * dy <= 1.0f;
			};

		CellList out;
		for (int y = minY; y <= maxY; ++y) {
			for (int x = minX; x <= maxX; ++x) {
				if (!inside(x, y)) continue;
				const bool edge = !inside(x - 1, y) || !inside(x + 1, y) || !inside(x, y - 1) || !inside(x, y + 1);
				if (filled || edge) out.push_back({ x, y });
			}
		}
		return out;
	}

	/// 塗りつぶし：(sx, sy) と同じ番号で上下左右につながっている所を value に。変えたマス数を返す
	int FloodFill(TileMapData& map, int sx, int sy, int value) {
		const int target = map.Row[sy][sx];
		if (target == value) return 0;                      // 同じ番号で塗ると無限に続くので何もしない

		int count = 0;
		std::vector<std::pair<int, int>> stack{ { sx, sy } };
		while (!stack.empty()) {
			const auto [x, y] = stack.back();
			stack.pop_back();
			if (!map.InBounds(x, y) || map.Row[y][x] != target) continue;
			map.Row[y][x] = value;
			++count;
			stack.push_back({ x + 1, y });
			stack.push_back({ x - 1, y });
			stack.push_back({ x, y + 1 });
			stack.push_back({ x, y - 1 });
		}
		return count;
	}

	Matrix4x4 MakeScreenMatrix(Matrix4x4 viewProj, const ImVec2& min, const ImVec2& size) {
		return viewProj * MakeViewportMatrix(min.x, min.y, size.x, size.y, 0.0f, 1.0f);
	}

	Vector3 CellToWorld(int x, int y, int height, const Vector2& sp) {
		return Vector3{ (x + 0.5f) * sp.x, (height - 1 - y + 0.5f) * sp.y, 0.0f };
	}

	bool ScreenToCell(const ImVec2& mouse, const Matrix4x4& toScreen, const TileMapData& map, const Vector2& sp, int& outX, int& outY) {
		const Matrix4x4 toWorld = Inverse(toScreen);
		Vector3 nearP = viewFinalTransform(Vector3{ mouse.x, mouse.y, 0.0f }, toWorld);
		Vector3 farP = viewFinalTransform(Vector3{ mouse.x, mouse.y, 1.0f }, toWorld);

		Segment ray{ nearP, farP - nearP };
		const Plane ground{ Vector3{ 0.0f, 0.0f, 1.0f }, 0.0f };      // Z = 0
		float t = 0.0f;
		if (!crashDecision(ray, ground, &t)) return false;

		Vector3 hit = nearP + ray.diff * t;
		outX = (int)floorf(hit.x / sp.x + 0.5f);
		outY = map.Height() - 1 - (int)floorf(hit.y / sp.y + 0.5f);
		return map.InBounds(outX, outY);
	}

	/// master側でImGuiのマルチビューポートが有効になったため、ウィンドウ座標はデスクトップ座標になる。
	/// このエディタのレイアウト計算はメインウィンドウ基準(左上=0,0)なので、ここで変換する。
	ImVec2 MainViewportOrigin() { return ImGui::GetMainViewport()->Pos; }
	void SetNextWindowPosLocal(const ImVec2& localPos) {
		const ImVec2 o = MainViewportOrigin();
		ImGui::SetNextWindowPos(ImVec2(o.x + localPos.x, o.y + localPos.y));
	}
	ImVec2 GetWindowPosLocal() {
		const ImVec2 o = MainViewportOrigin();
		const ImVec2 p = ImGui::GetWindowPos();
		return ImVec2(p.x - o.x, p.y - o.y);
	}
	/// LoadModelBranch が対応している拡張子だけ（*.* は入れない：非対応形式は空モデルになる）
	constexpr const char* kModelFilter = "Model (*.obj;*.gltf;*.glb)\0*.obj;*.gltf;*.glb\0";
	/// TextureManager は読めないと assert で止まるので画像だけ
	constexpr const char* kTextureFilter = "Image (*.png;*.jpg;*.dds)\0*.png;*.jpg;*.jpeg;*.dds\0";
}
#endif

/// for save load(しばらくのもの、後でクラス化にする)
namespace {
	constexpr int kFileVersion = 1;
	constexpr const char* kProjectFilter = "MapEditor Project (*.mmproj)\0*.mmproj\0";

	bool WriteJsonFile(const std::string& path, const nlohmann::json& j) {
		std::ofstream out(path);
		if (!out.is_open()) return false;
		out << j.dump(4);
		return true;
	}
	bool ReadJsonFile(const std::string& path, nlohmann::json& out) {
		std::ifstream in(path);
		if (!in.is_open()) return false;
		out = nlohmann::json::parse(in, nullptr, false);
		return !out.is_discarded();
	}

	nlohmann::json EngineParamsToJson(const EngineModelParams& p) {
		return {
			{ "sphereLat", p.sphereLat }, { "sphereLong", p.sphereLong },
			{ "ringDivide", p.ringDivide }, { "ringOuter", p.ringOuter }, { "ringInner", p.ringInner },
			{ "cylinderDivide", p.cylinderDivide }, { "cylinderTop", p.cylinderTop },
			{ "cylinderBottom", p.cylinderBottom }, { "cylinderHeight", p.cylinderHeight },
			{ "cylinderReverseY", p.cylinderReverseY },
		};
	}
	EngineModelParams JsonToEngineParams(const nlohmann::json& j) {
		EngineModelParams p{};
		p.sphereLat = j.value("sphereLat", p.sphereLat);
		p.sphereLong = j.value("sphereLong", p.sphereLong);
		p.ringDivide = j.value("ringDivide", p.ringDivide);
		p.ringOuter = j.value("ringOuter", p.ringOuter);
		p.ringInner = j.value("ringInner", p.ringInner);
		p.cylinderDivide = j.value("cylinderDivide", p.cylinderDivide);
		p.cylinderTop = j.value("cylinderTop", p.cylinderTop);
		p.cylinderBottom = j.value("cylinderBottom", p.cylinderBottom);
		p.cylinderHeight = j.value("cylinderHeight", p.cylinderHeight);
		p.cylinderReverseY = j.value("cylinderReverseY", p.cylinderReverseY);
		return p;
	}

	std::string MakeUniqueName(std::set<std::string>& used, const std::string& base) {
		std::string name = base;
		for (int n = 1; used.count(name); ++n) name = base + "_" + std::to_string(n);
		used.insert(name);
		return name;
	}
}

MapMakingEditor::MapMakingEditor(kEngine* system) {
	system_ = system;
	debugText_ = "";
	auto& roomMap = currentMapInfo_.tileMapData;
}

MapMakingEditor::~MapMakingEditor() {}

void MapMakingEditor::Update() {
	DeletePart();
}

void MapMakingEditor::Draw() {
	ImGuiPart();
}

#ifdef USE_IMGUI
void MapMakingEditor::ImGuiPart() {

	ImGuiIO& io = ImGui::GetIO();
	menuBarHeight_ = ImGui::GetFrameHeight();

	//ImGui::ShowDemoWindow();

	/// ============== メニューバー =============== ///
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {};
			if (ImGui::MenuItem("Load Project...")) {
				const std::string path = FileManager::OpenFileDialog(kProjectFilter, "Load Project");
				if (!path.empty()) LoadProject(path);
			}
			if (ImGui::MenuItem("Save Project")) {
				if (projectFilePath_.empty()) SaveProjectAs();
				else SaveProject(projectFilePath_);
			}
			if (ImGui::MenuItem("Save Project As...")) SaveProjectAs();
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
		ImGui::EndMainMenuBar();
	}

	/// ============ 左側のウィンドウ群 ============= ///

	ImGuiLeftMenuBar();

	/// ============ 右側のウィンドウ群 ============= ///

	ImGuiRightMenuBar();

	/// ========== 中央のマップウィンドウ ============ ///

	ImGuiMidWindow();


	/// ============ Popup Window ============= ///
	ImGuiLoadWindow();
	ImGuiSaveWindow();
	ImGuiDoubleCheckWindow();
	ImGuiMapNewWindow();
	ImGuiNewVariablesWindow();
	ImGuiEngineModelWindow();
}


void MapMakingEditor::DeletePart() {
	if (!isDelete_) return;
	switch (selectMode_) {
	case SelectModeForMME::TileMap:
		if (currentMapIndex_ >= 0 && currentMapIndex_ < (int)mapList_.size()) {
			mapList_.erase(mapList_.begin() + currentMapIndex_);
			currentMapIndex_ = -1;
		} else {
			debugText_ += "Error: Invalid currentMapIndex_ for delete.\n";
		}
		break;
	case SelectModeForMME::MapMaker:
		if (currentMakerIndex_ >= 0 && currentMakerIndex_ < static_cast<int>(mapMakerlist_.size())) {
			mapMakerlist_.erase(mapMakerlist_.begin() + currentMakerIndex_);
		} else {
			debugText_ += "Error: Invalid currentMakerIndex_ for delete.\n";
		}
		break;
	case SelectModeForMME::Variable:
	{
		// MapMaker index 安全檢查
		if (currentMakerIndex_ < 0 || currentMakerIndex_ >= mapMakerlist_.size()) {
			debugText_ += "Error: Invalid currentMakerIndex_ for variable delete.\n";
			break;
		}
		auto& list = mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList;
		// Variable index 安全檢查
		if (currentVariableIndex_ < 0 || currentVariableIndex_ >= list.size()) {
			debugText_ += "Error: Invalid currentVariableIndex_ for delete.\n";
			break;
		}
		// 執行刪除
		list.erase(list.begin() + currentVariableIndex_);
		// 刪除後修正 index
		if (currentVariableIndex_ >= list.size())
			currentVariableIndex_ = (int)list.size() - 1;

		if (list.empty())
			currentVariableIndex_ = -1;
		break;
	}
	case SelectModeForMME::MapData:
		if (currentMapDataIndex_ >= 0 && currentMapDataIndex_ < (int)mapDataList_.size()) {
			mapDataList_.erase(mapDataList_.begin() + currentMapDataIndex_);
			currentMapDataIndex_ = -1;
			currentUnitIndex_ = -1;
		} else {
			debugText_ += "Error: Invalid currentMapDataIndex_ for delete.\n";
		}
		break;

	case SelectModeForMME::MapUnit:
	{
		if (currentMapDataIndex_ < 0 || currentMapDataIndex_ >= (int)mapDataList_.size()) {
			debugText_ += "Error: Invalid currentMapDataIndex_ for unit delete.\n";
			break;
		}
		auto& units = mapDataList_[currentMapDataIndex_].units;
		if (currentUnitIndex_ < 0 || currentUnitIndex_ >= (int)units.size()) {
			debugText_ += "Error: Invalid currentUnitIndex_ for delete.\n";
			break;
		}
		units.erase(units.begin() + currentUnitIndex_);
		if (currentUnitIndex_ >= (int)units.size()) currentUnitIndex_ = (int)units.size() - 1;
		mapDataList_[currentMapDataIndex_].isSaved = false;
		break;
	}
	default:
		debugText_ += "Error: No Selection for delete.\n";
		break;
	}
	isDelete_ = false;
}

void MapMakingEditor::MapInfoLoadJson() {
	if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {
		auto& maker = mapMakerlist_[currentMakerIndex_];
		currentMapInfo_.jsonData = maker.GetMapMakerJson();
	}
}

void MapMakingEditor::SyncUnitObject(std::unique_ptr<Object>& obj, const MapUnit& unit) {

	/// モデルが無い → プレビューしない
	if (unit.modelHandle < 0) {
		obj.reset();
		return;
	}

	/// まだ無い / モデルが変わった → 作り直す（CreateModelData は重いので必要な時だけ）
	if (!obj || obj->modelHandle_ != unit.modelHandle) {
		obj = std::make_unique<Object>();
		obj->IntObject(system_);
		if (unit.engineModel == EngineModelType::None) {
			obj->CreateModelData(unit.modelHandle);
		} else {
			obj->CreateDefaultData();            // 内蔵モデルは ModelData が無いので CreateModelData は使えない
			obj->modelHandle_ = unit.modelHandle;
		}
	}

	/// Transform / Material は値をコピーするだけなので毎フレームでOK（Detail の変更が即反映）
	obj->mainPosition.transform = unit.transform;

	for (int p = 0; p < (int)obj->objectParts_.size(); ++p) {
		MaterialConfig& mat = *obj->objectParts_[p].materialConfig;
		mat = unit.material;
		if (unit.material.useModelTexture && unit.engineModel == EngineModelType::None) {
			mat.textureHandle = system_->GetModelTextureHandle(unit.modelHandle, p);
		}
	}

}

void MapMakingEditor::FocusPreviewCamera(const MapUnit& unit) {
	const Vector3 target = unit.transform.translate;
	const float dist = 4.0f, height = 1.0f;
	previewCamPos_ = Vector3{ target.x, target.y + height, target.z - dist };
	previewCamRot_ = Vector3{ atan2f(height, dist), 0.0f, 0.0f };
}

bool MapMakingEditor::UpdateFreeCamera(Vector3& pos, Vector3& rot, float speed) {
	ImGuiIO& io = ImGui::GetIO();
	const bool hovered = ImGui::IsItemHovered();
	const bool active = ImGui::IsItemActive();
	if (!(hovered || active)) return false;

	bool focus = false;
	Vector3 move{ 0.0f, 0.0f, 0.0f };

	/// キーボード
	if (!io.WantTextInput) {
		const float step = 0.6f * speed * io.DeltaTime;
		if (ImGui::IsKeyDown(ImGuiKey_Q)) move.z += step;
		if (ImGui::IsKeyDown(ImGuiKey_E)) move.z -= step;
		if (ImGui::IsKeyDown(ImGuiKey_A)) move.x -= step;
		if (ImGui::IsKeyDown(ImGuiKey_D)) move.x += step;
		if (ImGui::IsKeyDown(ImGuiKey_W)) move.y += step;
		if (ImGui::IsKeyDown(ImGuiKey_S)) move.y -= step;
		if (ImGui::IsKeyPressed(ImGuiKey_R, false)) focus = true;
	}
	/// 右ドラッグ：視点回転
	if (active && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
		rot.x += io.MouseDelta.y * 0.005f;
		rot.y += io.MouseDelta.x * 0.005f;
		rot.x = std::clamp(rot.x, -1.55f, 1.55f);
	}
	/// 中ドラッグ：平行移動
	if (active && ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
		move.x += io.MouseDelta.x * -0.005f * speed;
		move.y += io.MouseDelta.y * 0.005f * speed;
	}
	/// ホイール：前後
	move.z += io.MouseWheel * 0.5f * speed;

	/// ローカル移動をカメラの向きでワールドに変換
	Matrix4x4 handle = MakeTranslateMatrix4x4(move) * MakeRotateMatrix4x4(rot);
	pos = pos + MakeTranslateVector3(handle);
	return focus;
}

void MapMakingEditor::ImGuiObjectPreview(MapData* data) {
	const bool valid = data && currentUnitIndex_ >= 0 && currentUnitIndex_ < (int)data->units.size();
	if (!valid) {
		previewObject_.reset();
		previewFocusIndex_ = -2;
		ImGui::TextDisabled(data ? "Select an object from Object List." : "Select a MapData from MapData List.");
		return;
	}

	MapUnit& unit = data->units[currentUnitIndex_];

	SyncUnitObject(unit);

	/// 対象が変わったらカメラを移す
	if (currentUnitIndex_ != previewFocusIndex_ || currentMapDataIndex_ != previewMapDataIndex_) {
		previewFocusIndex_ = currentUnitIndex_;
		previewMapDataIndex_ = currentMapDataIndex_;
		FocusPreviewCamera(unit);
	}

	if (!previewObject_) {
		ImGui::TextDisabled("This object has no model. Load a model in Object Detail.");
		return;
	}

	/// 表示サイズ：previewRT_ は 512x384（4:3）→ ウィンドウに収まる最大の 4:3 を中央に
	const ImVec2 avail = ImGui::GetContentRegionAvail();
	float w = avail.x;
	float h = w * 0.75f;
	if (h > avail.y) { h = avail.y; w = h / 0.75f; }
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail.x - w) * 0.5f);

	const ImVec2 p = ImGui::GetCursorScreenPos();
	DrawEngine* de = system_->GetDrawEngine();

	ImGui::Image((ImTextureID)de->GetPreviewTextureID(), ImVec2(w, h));
	ImGui::SetCursorScreenPos(p);
	ImGui::InvisibleButton("##objectPreview", ImVec2(w, h),
		ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
	if (UpdateFreeCamera(previewCamPos_, previewCamRot_, 1.0f)) FocusPreviewCamera(unit);  // ← InvisibleButton の直後に呼ぶ

	/// 左上：物件名、左下：操作説明
	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->AddText(ImVec2(p.x + 8.0f, p.y + 6.0f), IM_COL32(170, 180, 198, 255), unit.name.c_str());
	dl->AddText(ImVec2(p.x + 8.0f, p.y + h - 20.0f), IM_COL32(120, 130, 150, 255),
		"RMB: look   MMB: pan   Wheel: zoom   WASDQE: move   R: focus");

	/// DrawEngine に描画を申請（フレームの最後に previewRT_ へ描かれる）
	DrawEngine::PreviewRequest req;
	req.active = true;
	req.object = previewObject_.get();
	req.partIndex = -1;                              // 全パーツ
	req.view.view = Inverse(MakeAffineMatrix(Vector3{ 1.0f, 1.0f, 1.0f }, previewCamRot_, previewCamPos_));
	req.view.proj = MakePerspectiveFovMatrix(0.7f, 512.0f / 384.0f, 0.05f, 500.0f);
	req.view.position = previewCamPos_;
	de->SetPreviewRequest(req);
}

/// MapData の ID を変える + 開いているマップのマスも同じように入れ替える（見た目が変わらないように）
void MapMakingEditor::ChangeUnitId(MapData& data, int oldId, int newId) {
	if (newId < 0 || oldId == newId) return;
	data.ChangeId(oldId, newId);
	data.isSaved = false;

	for (auto& m : mapList_) {
		if (m.mapDataName != data.dataName) continue;
		bool touched = false;
		for (auto& row : m.tileMapData.Row) {
			for (int& v : row) {
				if (v == oldId) { v = newId; touched = true; } else if (v == newId) { v = oldId; touched = true; }
			}
		}
		if (touched) m.isSaved = false;
	}
	if (paintUnitId_ == oldId) paintUnitId_ = newId;
	else if (paintUnitId_ == newId) paintUnitId_ = oldId;
}

void MapMakingEditor::FocusMapCamera(const TileMapData& map, const Vector2& sp) {
	const float w = map.Width() * sp.x;
	const float h = map.Height() * sp.y;
	const float dist = (std::max)(w * 1.03f, h * 1.37f) * 1.05f + (std::max)(sp.x, sp.y);
	mapCamPos_ = Vector3{ w * 0.5f, h * 0.5f, -dist };
	mapCamRot_ = Vector3{ 0.0f, 0.0f, 0.0f };
}

void MapMakingEditor::ImGuiMapListWindow(ImVec2 size, ImVec2* outputSize, ImVec2 pos, ImVec2* outputPos, float colimenH) {
	SetNextWindowPosLocal(ImVec2(0, size.y + pos.y));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, colimenH));      // MapData List + Object List と同じ高さ

	ImGui::Begin("Map List", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	const bool hasMap = currentMapIndex_ >= 0 && currentMapIndex_ < (int)mapList_.size();

	switch (ImGuiWidgets::ListToolbar(leftInspectorWidth_ - 20)) {
	case ImGuiWidgets::ListButton::New: {
		MapInfo m{};
		m.tileMapData.name = "new_Map";
		if (newMapCounter_ != 0) m.tileMapData.name += "_" + std::to_string(newMapCounter_);
		newMapCounter_++;

		int w = kDefaultMapW, h = kDefaultMapH, blank = 0;
		if (currentMapDataIndex_ >= 0 && currentMapDataIndex_ < (int)mapDataList_.size()) {
			const MapData& d = mapDataList_[currentMapDataIndex_];      // 今選んでいる MapData を使う
			m.mapDataName = d.dataName;
			if (d.xSize > 0) w = d.xSize;
			if (d.ySize > 0) h = d.ySize;
			blank = d.blankId;
		}
		m.tileMapData.Resize(w, h, 0);
		if (hasMap) m.savePath = mapList_[currentMapIndex_].savePath;
		mapList_.push_back(m);
		SelectMap((int)mapList_.size() - 1);
		break;
	}
	case ImGuiWidgets::ListButton::Double:
		if (hasMap) {
			MapInfo copy = mapList_[currentMapIndex_];
			copy.tileMapData.name += "_copy";
			copy.isSaved = false;
			mapList_.push_back(copy);
			SelectMap((int)mapList_.size() - 1);
		} else debugText_ += "Error: No map selected for doubling.\n";
		break;

	case ImGuiWidgets::ListButton::Save: {
		if (!hasMap) { debugText_ += "Error: No map selected for save.\n"; break; }

		MapInfo& m = mapList_[currentMapIndex_];
		std::string path = m.savePath;

		if (path.empty()) path = FileManager::SaveFileDialog("CSV (*.csv)\0*.csv\0", "csv",
			FileManager::ToSafeFileName(m.tileMapData.name) + ".csv", "Save Map CSV");
		if (path.empty()) break;

		if (m.tileMapData.ExportCsv(path, m.mapDataName)) {
			m.savePath = path;
			m.isSaved = true;
			debugText_ += "Map saved: " + path + "\n";
		} else {
			debugText_ += "Save Error: " + path + "\n";
		}

		break;
	}
	case ImGuiWidgets::ListButton::Load: {
		const std::string path = FileManager::OpenFileDialog("CSV (*.csv)\0*.csv\0", "Load Map CSV");
		if (path.empty()) break;

		MapInfo m{};
		std::string dataName;
		if (!m.tileMapData.ImportCsv(path, &dataName)) {
			debugText_ += "Load Error: " + path + "\n";
			break;
		}
		m.tileMapData.name = std::filesystem::path(path).stem().string();
		/// csv に名前が無ければ、今選んでいる MapData を使う
		if (dataName.empty() && currentMapDataIndex_ >= 0 && currentMapDataIndex_ < (int)mapDataList_.size()) {
			dataName = mapDataList_[currentMapDataIndex_].dataName;
		}
		m.mapDataName = dataName;
		if (!dataName.empty() && !FindMapData(dataName)) {
			debugText_ += "Warning: MapData '" + dataName + "' is not loaded. Load it to see the map.\n";
		}
		m.savePath = path;
		m.isSaved = true;
		mapList_.push_back(m);
		SelectMap((int)mapList_.size() - 1);
		break;
	}
	case ImGuiWidgets::ListButton::Delete:
		doubleCheck_ = true;
		selectMode_ = SelectModeForMME::TileMap;
		break;
	default: break;
	}

	/// ファイルごとにまとめて表示（出てきた順）
	ImGui::BeginChild("MapList", ImVec2(0, ImGui::GetContentRegionAvail().y), true);
	std::vector<std::string> files;

	for (int i = 0; i < (int)mapList_.size(); ++i) {
		const MapInfo& m = mapList_[i];
		std::string label = m.tileMapData.name + (m.isSaved ? "" : " *");
		if (!m.mapDataName.empty()) label += "  [" + m.mapDataName + "]";
		ImGui::PushID(i);
		if (ImGui::Selectable(label.c_str(), currentMapIndex_ == i)) SelectMap(i);
		ImGui::PopID();
	}

	ImGui::EndChild();

	isMapDataSelected_ = false;          // 右側パネルは Map Detail を出す
	isMakerSelected_ = false;

	*outputPos = GetWindowPosLocal();
	*outputSize = ImGui::GetWindowSize();
	ImGui::End();
}

void MapMakingEditor::SelectMap(int index) {
	currentMapIndex_ = index;
	selectMode_ = SelectModeForMME::TileMap;
	const TileMapData& map = mapList_[index].tileMapData;
	mapResizeBuf_[0] = map.Width();
	mapResizeBuf_[1] = map.Height();
}

MapData* MapMakingEditor::FindMapData(const std::string& name) {
	for (auto& d : mapDataList_) if (d.dataName == name) return &d;
	return nullptr;
}

int MapMakingEditor::GetEngineModelHandle(EngineModelType type, const EngineModelParams& p) {
	const EngineModelParams def{};

	switch (type) {
	case EngineModelType::Plane:    return config::default_Plane_MeshBufferHandle_;
	case EngineModelType::Triangle: return config::default_Triangle_MeshBufferHandle_;
	case EngineModelType::Cube:     return config::default_Cube_MeshBufferHandle_;

	case EngineModelType::Sphere: {
		if (p.sphereLat == def.sphereLat && p.sphereLong == def.sphereLong) return config::default_Sphere_MeshBufferHandle_;
		const std::string key = "Sphere_" + std::to_string(p.sphereLat) + "_" + std::to_string(p.sphereLong);
		if (auto it = engineModelCache_.find(key); it != engineModelCache_.end()) return it->second;
		SphereBuildMaterial m;
		m.LatitudeSegments = p.sphereLat;
		m.LongitudeSegments = p.sphereLong;
		return engineModelCache_[key] = system_->CreateEngineModel(m);
	}
	case EngineModelType::Ring: {
		if (p.ringDivide == def.ringDivide && p.ringOuter == def.ringOuter && p.ringInner == def.ringInner)
			return config::default_Ring_MeshBufferHandle_;
		const std::string key = "Ring_" + std::to_string(p.ringDivide) + "_" + std::to_string(p.ringOuter) + "_" + std::to_string(p.ringInner);
		if (auto it = engineModelCache_.find(key); it != engineModelCache_.end()) return it->second;
		RingBuildMaterial m;
		m.Subdivision = p.ringDivide;
		m.OuterRadius = p.ringOuter;
		m.InnerRadius = p.ringInner;
		return engineModelCache_[key] = system_->GetResourceManager()->CreateEngineModel(m);   // kEngine に Ring 版が無い
	}
	case EngineModelType::Cylinder: {
		if (p.cylinderDivide == def.cylinderDivide && p.cylinderTop == def.cylinderTop &&
			p.cylinderBottom == def.cylinderBottom && p.cylinderHeight == def.cylinderHeight && !p.cylinderReverseY)
			return config::default_Cylinder_MeshBufferHandle_;
		const std::string key = "Cylinder_" + std::to_string(p.cylinderDivide) + "_" + std::to_string(p.cylinderTop) + "_" +
			std::to_string(p.cylinderBottom) + "_" + std::to_string(p.cylinderHeight) + "_" + std::to_string(p.cylinderReverseY);
		if (auto it = engineModelCache_.find(key); it != engineModelCache_.end()) return it->second;
		CylinderBuildMaterial m;
		m.Division = p.cylinderDivide;
		m.TopRadius = p.cylinderTop;
		m.BottomRadius = p.cylinderBottom;
		m.Height = p.cylinderHeight;
		m.isReverseY = p.cylinderReverseY;
		return engineModelCache_[key] = system_->CreateEngineModel(m);
	}
	default: return -1;
	}
}

void MapMakingEditor::ImGuiEngineModelWindow() {

	/// 予約されていたら開く（OpenPopup と BeginPopupModal は同じ ID スタックで呼ぶ必要がある）
	if (showEngineModelWindow_) {
		ImGui::OpenPopup("Engine Model");
		showEngineModelWindow_ = false;
	}

	if (!ImGui::BeginPopupModal("Engine Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		enginePreviewObject_.reset();          // 閉じている間はプレビュー用 Object も持たない
		return;
	}

	EngineModelParams& p = engineEditParams_;
	bool commit = false;                       // 入力を確定した（ドラッグを離した / Enter）

	/// ================= 左：種類と数値 ================= ///
	ImGui::BeginGroup();
	ImGui::SeparatorText("Model");
	for (const auto& [type, name] : EnumNameMap<EngineModelType>) {
		if (ImGui::RadioButton(name.c_str(), engineSelectType_ == type)) engineSelectType_ = type;
	}

	ImGui::SeparatorText("Parameters");
	ImGui::PushItemWidth(160.0f);
	switch (engineSelectType_) {
	case EngineModelType::Sphere:
		ImGui::DragInt("Latitude", &p.sphereLat, 1, 3, 128);       commit |= ImGui::IsItemDeactivatedAfterEdit();
		ImGui::DragInt("Longitude", &p.sphereLong, 1, 3, 128);     commit |= ImGui::IsItemDeactivatedAfterEdit();
		break;
	case EngineModelType::Ring:
		ImGui::DragInt("Divide", &p.ringDivide, 1, 3, 256);        commit |= ImGui::IsItemDeactivatedAfterEdit();
		ImGui::DragFloat("Outer", &p.ringOuter, 0.01f, 0.01f, 100.0f);         commit |= ImGui::IsItemDeactivatedAfterEdit();
		ImGui::DragFloat("Inner", &p.ringInner, 0.01f, 0.0f, p.ringOuter);     commit |= ImGui::IsItemDeactivatedAfterEdit();
		break;
	case EngineModelType::Cylinder:
		ImGui::DragInt("Divide", &p.cylinderDivide, 1, 3, 256);    commit |= ImGui::IsItemDeactivatedAfterEdit();
		ImGui::DragFloat("Top", &p.cylinderTop, 0.01f, 0.0f, 100.0f);          commit |= ImGui::IsItemDeactivatedAfterEdit();
		ImGui::DragFloat("Bottom", &p.cylinderBottom, 0.01f, 0.0f, 100.0f);    commit |= ImGui::IsItemDeactivatedAfterEdit();
		ImGui::DragFloat("Height", &p.cylinderHeight, 0.01f, 0.01f, 100.0f);   commit |= ImGui::IsItemDeactivatedAfterEdit();
		commit |= ImGui::Checkbox("Reverse Y", &p.cylinderReverseY);
		break;
	default:
		ImGui::TextDisabled("No parameters (engine default mesh).");
		break;
	}
	ImGui::PopItemWidth();
	if (ImGui::Button("Reset Parameters")) {
		p = EngineModelParams{};
		commit = true;
	}
	if (commit) enginePreviewParams_ = p;      // 確定した値だけをプレビュー・作成に使う
	ImGui::EndGroup();

	ImGui::SameLine();

	/// ================= 右：小さいプレビュー ================= ///
	ImGui::BeginGroup();
	const int handle = GetEngineModelHandle(engineSelectType_, enginePreviewParams_);

	/// モデルが変わったら作り直す（内蔵モデルは CreateModelData ではなく CreateDefaultData）
	if (!enginePreviewObject_ || enginePreviewObject_->modelHandle_ != handle) {
		enginePreviewObject_ = std::make_unique<Object>();
		enginePreviewObject_->IntObject(system_);
		enginePreviewObject_->CreateDefaultData();
		enginePreviewObject_->modelHandle_ = handle;
		enginePreviewObject_->objectParts_[0].materialConfig->textureHandle = config::default_White5x5_TextureHandle_;
		enginePreviewObject_->objectParts_[0].materialConfig->lightModelType = LightModelType::Lambert;
	}
	/// 片面しかないモデル（裏から見ると消える）は回さず、正面（-Z = カメラ側）を向けたまま
	const bool singleSided = (engineSelectType_ == EngineModelType::Plane || engineSelectType_ == EngineModelType::Ring);
	if (engineSelectType_ == EngineModelType::Ring) {
		enginePreviewObject_->mainPosition.transform.rotate.y = 0.0f;
	} else {
		enginePreviewObject_->mainPosition.transform.rotate.y += ImGui::GetIO().DeltaTime * 0.8f;
	}

	DrawEngine* de = system_->GetDrawEngine();
	ImGui::Image((ImTextureID)de->GetPreviewTextureID(), ImVec2(256.0f, 192.0f));

	const float dist = (engineSelectType_ == EngineModelType::Cylinder) ? 6.0f : 4.0f;
	const Vector3 camPos{ 0.0f, 1.5f, -dist };
	const Vector3 camRot{ atan2f(1.5f, dist), 0.0f, 0.0f };

	DrawEngine::PreviewRequest req;
	req.active = true;
	req.object = enginePreviewObject_.get();
	req.partIndex = -1;
	req.view.view = Inverse(MakeAffineMatrix(Vector3{ 1.0f, 1.0f, 1.0f }, camRot, camPos));
	req.view.proj = MakePerspectiveFovMatrix(0.7f, 512.0f / 384.0f, 0.05f, 500.0f);
	req.view.position = camPos;
	de->SetPreviewRequest(req);              // 中央プレビューより後に呼ばれるので、こちらが優先される
	ImGui::EndGroup();

	/// ================= 下：決定 / キャンセル ================= ///
	ImGui::Separator();
	if (ImGui::Button("Use", ImVec2(120.0f, 0.0f))) {
		enginePreviewParams_ = p;            // 確定前の入力も反映してから使う
		const bool valid = engineTargetMapData_ >= 0 && engineTargetMapData_ < (int)mapDataList_.size() &&
			engineTargetUnit_ >= 0 && engineTargetUnit_ < (int)mapDataList_[engineTargetMapData_].units.size();
		if (valid) {
			MapUnit& unit = mapDataList_[engineTargetMapData_].units[engineTargetUnit_];
			unit.modelHandle = GetEngineModelHandle(engineSelectType_, enginePreviewParams_);
			unit.engineModel = engineSelectType_;
			unit.engineParams = enginePreviewParams_;
			unit.material.useModelTexture = (unit.engineModel == EngineModelType::None);
			unit.material.textureHandle = config::default_White5x5_TextureHandle_;
			mapDataList_[engineTargetMapData_].isSaved = false;
		}
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel", ImVec2(120.0f, 0.0f))) {
		ImGui::CloseCurrentPopup();
	}

	ImGui::EndPopup();

}

std::string MapMakingEditor::GetModelPath(int modelHandle) const {
	auto& groups = system_->GetResourceManager()->modelGroupList_;
	if (modelHandle < 0 || modelHandle >= (int)groups.size()) return "";
	return groups[modelHandle]->GetFilePath();
}

nlohmann::json MapMakingEditor::UnitToJson(const MapUnit& u, const std::string& baseDir) const {
	nlohmann::json j;
	j["id"] = u.id;
	j["name"] = u.name;
	j["isDirectTile"] = u.isDirectTile;
	j["previewColor"] = JsonHelper::V4ToJson(u.previewColor);

	nlohmann::json model;                       // handle は保存しない（起動ごとに変わる）
	if (u.engineModel != EngineModelType::None) {
		model["type"] = "engine";
		model["engine"] = GetEnumName(u.engineModel);
		model["params"] = EngineParamsToJson(u.engineParams);
	} else if (u.modelHandle >= 0) {
		model["type"] = "file";
		model["path"] = ToSavePath(GetModelPath(u.modelHandle), baseDir);
	} else {
		model["type"] = "none";
	}
	j["model"] = model;

	j["transform"] = JsonHelper::TransformToJson(u.transform);

	/// material は1回だけ作って、texture のパスを後から差し替える
	j["material"] = JsonHelper::MaterialConfigToJson(u.material, system_);
	if (u.material.useModelTexture) {
		j["material"]["textureHandle"] = "";                                  // モデル付属 → 持たない
	} else {
		j["material"]["textureHandle"] = ToSavePath(j["material"].value("textureHandle", std::string()), baseDir);
	}
	return j;
}

MapUnit MapMakingEditor::JsonToUnit(const nlohmann::json& j, const std::string& baseDir) {
	MapUnit u{};
	u.id = j.value("id", 1);
	u.name = j.value("name", std::string("NewUnit"));
	u.isDirectTile = j.value("isDirectTile", true);
	if (j.contains("previewColor")) u.previewColor = JsonHelper::JsonToV4(j["previewColor"]);
	if (j.contains("transform"))    u.transform = JsonHelper::JsonToTransform(j["transform"]);
	bool texturePathEmpty = true;
	if (j.contains("material")) {
		nlohmann::json mat = j["material"];
		// 相対路径を元のパスに変換
		std::string texPath = FileManager::ResolveFrom(mat.value("textureHandle", std::string()), baseDir);
		mat["textureHandle"] = texPath;
		/// ファイルが無い texture を読むと TextureManager が assert で止まる → 空にして白で代用
		if (!texPath.empty() && !std::filesystem::exists(texPath)) {
			debugText_ += "Load Warning: texture not found " + texPath + "\n";
			mat["textureHandle"] = "";
			texPath.clear();
		}
		texturePathEmpty = texPath.empty();
		u.material = JsonHelper::JsonToMaterialConfig(mat, system_);
	}

	const nlohmann::json model = j.value("model", nlohmann::json::object());
	const std::string type = model.value("type", std::string("none"));

	if (type == "file") {
		const std::string path = FileManager::ResolveFrom(model.value("path", std::string()), baseDir);
		if (!path.empty()) {
			const int handle = system_->SetModelObj(path);
			if (system_->GetMutiModelNum(handle) > 0) u.modelHandle = handle;
			else debugText_ += "Load Warning: cannot load model " + path + "\n";
		}
	} else if (type == "engine") {
		const std::string engineName = model.value("engine", std::string());
		for (const auto& [t, name] : EnumNameMap<EngineModelType>) {
			if (name == engineName) u.engineModel = t;
		}
		u.engineParams = JsonToEngineParams(model.value("params", nlohmann::json::object()));
		if (u.engineModel != EngineModelType::None) u.modelHandle = GetEngineModelHandle(u.engineModel, u.engineParams);
	}

	/// texture の後始末
	if (u.material.useModelTexture && u.engineModel == EngineModelType::None && u.modelHandle >= 0) {
		u.material.textureHandle = system_->GetModelTextureHandle(u.modelHandle, 0);   // モデル付属を取り直す
	} else if (texturePathEmpty) {
		u.material.textureHandle = config::default_White5x5_TextureHandle_;           // パスが空だった → 白
	}
	return u;
}

nlohmann::json MapMakingEditor::MapDataToJson(const MapData& d, const std::string& baseDir) const {
	nlohmann::json j;
	j["kind"] = "MapData";
	j["version"] = kFileVersion;
	j["name"] = d.dataName;
	j["xSize"] = d.xSize;
	j["ySize"] = d.ySize;
	j["startX"] = d.startX;
	j["startY"] = d.startY;
	j["units"] = nlohmann::json::array();
	j["blankId"] = d.blankId;
	j["cellSpacing"] = { d.cellSpacing.x, d.cellSpacing.y };
	for (const auto& u : d.units) j["units"].push_back(UnitToJson(u, baseDir));
	return j;
}

MapData MapMakingEditor::JsonToMapData(const nlohmann::json& j, const std::string& baseDir) {
	MapData d{};
	d.dataName = j.value("name", std::string("new_MapData"));
	d.xSize = j.value("xSize", 0);
	d.ySize = j.value("ySize", 0);
	d.startX = j.value("startX", 0);
	d.startY = j.value("startY", 0);
	d.blankId = j.value("blankId", 0);
	if (j.contains("cellSpacing") && j["cellSpacing"].size() >= 2) {
		d.cellSpacing = { j["cellSpacing"][0].get<float>(), j["cellSpacing"][1].get<float>() };
	}
	for (const auto& uj : j.value("units", nlohmann::json::array())) d.units.push_back(JsonToUnit(uj, baseDir));
	d.isSaved = true;
	return d;
}

bool MapMakingEditor::SaveProject(const std::string& projectPath) {
	namespace fs = std::filesystem;
	if (currentMakerIndex_ >= 0 && currentMakerIndex_ < (int)mapMakerlist_.size()) {
		mapMakerlist_[currentMakerIndex_].SetCodingText(codeingText_);   // 編集中の腳本を戻す
	}

	const fs::path root = fs::path(projectPath).parent_path();
	std::error_code ec;
	fs::create_directories(root / "MapData", ec);
	fs::create_directories(root / "MapMaker", ec);
	if (ec) debugText_ += "Folder Error: " + ec.message() + "\n";

	nlohmann::json project;
	project["kind"] = "MapEditorProject";
	project["version"] = kFileVersion;
	project["mapData"] = nlohmann::json::array();
	project["mapMaker"] = nlohmann::json::array();
	project["relativePaths"] = useRelativePath_;

	std::set<std::string> usedData, usedMaker;

	for (auto& d : mapDataList_) {
		const std::string file = "MapData/" + MakeUniqueName(usedData, FileManager::ToSafeFileName(d.dataName)) + ".json";
		if (!WriteJsonFile((root / file).string(), MapDataToJson(d, (root / "MapData").string()))) {
			debugText_ += "Save Error: " + file + "\n";
			return false;
		}
		d.isSaved = true;
		project["mapData"].push_back(file);
	}

	fs::create_directories(root / "Maps", ec);
	project["maps"] = nlohmann::json::array();
	std::set<std::string> usedMap;
	for (auto& m : mapList_) {
		const std::string file = "Maps/" + MakeUniqueName(usedMap, FileManager::ToSafeFileName(m.tileMapData.name)) + ".csv";
		if (!m.tileMapData.ExportCsv((root / file).string(), m.mapDataName)) {
			debugText_ += "Save Error: " + file + "\n";
			return false;
		}
		m.savePath = (root / file).string();
		m.isSaved = true;
		project["maps"].push_back(file);
	}

	for (auto& m : mapMakerlist_) {
		m.SetMapName(MakeUniqueName(usedMaker, FileManager::ToSafeFileName(m.GetMapName())));  // 同名だと同じファイルに上書きされる
		m.LoadScript(m.GetCodingText());
		m.Compile();
		m.BuildJson();
		if (!m.ExportMapMakerToJson((root / "MapMaker").string())) {
			debugText_ += "Save Error: " + m.GetDebugText() + "\n";
			return false;
		}
		m.SetSaved(true);
		project["mapMaker"].push_back("MapMaker/" + m.GetMapName() + ".json");
	}

	if (!WriteJsonFile(projectPath, project)) {
		debugText_ += "Save Error: " + projectPath + "\n";
		return false;
	}
	projectFilePath_ = projectPath;
	debugText_ += "Project saved: " + projectPath + "\n";
	return true;
}

bool MapMakingEditor::LoadProject(const std::string& projectPath) {
	namespace fs = std::filesystem;

	nlohmann::json project;
	if (!ReadJsonFile(projectPath, project) || project.value("kind", std::string()) != "MapEditorProject") {
		debugText_ += "Load Error: not a project file: " + projectPath + "\n";
		return false;
	}
	const fs::path root = fs::path(projectPath).parent_path();

	std::vector<MapData> datas;          // 一旦別の入れ物に（途中で失敗しても今のデータを壊さない）
	std::vector<MapMaker> makers;
	std::vector<MapInfo> maps;

	for (const auto& f : project.value("maps", nlohmann::json::array())) {
		const std::string full = (root / f.get<std::string>()).string();
		MapInfo m{};
		if (m.tileMapData.ImportCsv(full, &m.mapDataName)) {
			m.tileMapData.name = std::filesystem::path(full).stem().string();
			m.savePath = full;
			m.isSaved = true;
			maps.push_back(std::move(m));
		} else {
			debugText_ += "Load Warning: cannot read " + full + "\n";
		}
	}
	for (const auto& f : project.value("mapData", nlohmann::json::array())) {
		const std::string file = f.get<std::string>();
		nlohmann::json j;
		if (ReadJsonFile((root / file).string(), j)) datas.push_back(JsonToMapData(j, (root / "MapData").string()));
		else debugText_ += "Load Warning: cannot read " + file + "\n";
	}
	for (const auto& f : project.value("mapMaker", nlohmann::json::array())) {
		const std::string file = f.get<std::string>();
		MapMaker maker;
		if (maker.ImportMapMakerFromJson((root / file).string())) makers.push_back(maker);
		if (!maker.GetDebugText().empty()) debugText_ += maker.GetDebugText() + "\n";
	}

	mapDataList_ = std::move(datas);
	mapMakerlist_ = std::move(makers);
	mapList_ = std::move(maps);          // mapDataList_ / mapMakerlist_ と一緒に入れ替え
	currentMapIndex_ = -1;
	currentMapDataIndex_ = -1;
	currentUnitIndex_ = -1;
	currentMakerIndex_ = -1;
	currentVariableIndex_ = -1;
	codeingText_.clear();
	previewObject_.reset();
	selectMode_ = SelectModeForMME::None;
	useRelativePath_ = project.value("relativePaths", true);     // 古いプロジェクトは相対

	projectFilePath_ = projectPath;
	debugText_ += "Project loaded: " + projectPath + "\n";
	return true;
}

void MapMakingEditor::SaveProjectAs() {
	const std::string path = FileManager::SaveFileDialog(kProjectFilter, "mmproj", "project.mmproj", "Save Project (choose folder)");
	if (!path.empty()) SaveProject(path);
}

std::string MapMakingEditor::ToSavePath(const std::string& path, const std::string& baseDir) const {
	return useRelativePath_ ? FileManager::MakeRelativeTo(path, baseDir)
		: FileManager::ToAbsolutePath(path);
}

void MapMakingEditor::ImGuiLeftMenuBar() {

	ImGuiIO& io = ImGui::GetIO();

	float selectorH = menuBarHeight_;
	float lineH = ImGui::GetTextLineHeightWithSpacing();
	float titleH = ImGui::GetFrameHeight();

	float templateLines = 1;
	float templateH = templateLines * lineH + 20.0f + titleH;

	float selectorPosY = menuBarHeight_;
	float templatePosY = io.DisplaySize.y - templateH;

	ImVec2 w1Pos{};
	ImVec2 w1Size{};

	/// =========================
	/// 1. Mode Selector
	/// =========================
	SetNextWindowPosLocal(ImVec2(0, selectorPosY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, selectorH));

	ImGui::Begin("##LeftSelector",
		nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse);

	if (ImGui::Selectable("MakerEditor", modeEditing_ == 0, 0, ImVec2(80, selectorH - 4)))
		modeEditing_ = 0;
	ImGui::SameLine();
	if (ImGui::Selectable("RoomEditor", modeEditing_ == 1, 0, ImVec2(80, selectorH - 4)))
		modeEditing_ = 1;
	ImGui::SameLine();
	if (ImGui::Selectable("MapGenerator", modeEditing_ == 2, 0, ImVec2(80, selectorH - 4)))
		modeEditing_ = 2;
	ImGui::SameLine();

	w1Pos = GetWindowPosLocal();
	w1Size = ImGui::GetWindowSize();

	ImGui::End();

	/// =========================
	/// 2. Combo MapData \ MapMaker（補償用選擇器）
	/// =========================
	float window2Y = w1Pos.y + w1Size.y;
	ImVec2 w2Pos;
	ImVec2 w2Size;

	SetNextWindowPosLocal(ImVec2(0, window2Y));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, 0)); // AutoResize

	isMapDataWindowOpen_ = ImGui::Begin("##LeftDataSelector",
		nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::SetNextItemWidth(leftInspectorWidth_ - 20);

	/// ------------------------------------------------------------
	/// 如果 Tag 選的是 MapData → 這裡顯示 MapMaker 的 list
	/// ------------------------------------------------------------
	if (!IsGeneratorMode() && selectModeForLeftData_ == SelectModeForLeftData::MapData) {
		// 目前選擇的 MapMaker 名稱
		const char* preview = "None";
		if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size())
			preview = mapMakerlist_[currentMakerIndex_].GetMapName().c_str();

		if (ImGui::BeginCombo("##MakerSelector", preview)) {
			for (int i = 0; i < mapMakerlist_.size(); i++) {
				auto& maker = mapMakerlist_[i];
				bool selected = (currentMakerIndex_ == i);

				if (ImGui::Selectable(maker.GetMapName().c_str(), selected)) {
					currentMakerIndex_ = i;
				}
			}
			ImGui::EndCombo();
		}
	}

	/// ------------------------------------------------------------
	/// 如果 Tag 選的是 MapMaker → 這裡顯示 MapData 的 list
	/// ------------------------------------------------------------
	else {
		const char* preview = "None";
		if (currentMapDataIndex_ >= 0 && currentMapDataIndex_ < mapDataList_.size())
			preview = mapDataList_[currentMapDataIndex_].dataName.c_str();

		if (ImGui::BeginCombo("##DataSelector", preview)) {
			for (int i = 0; i < mapDataList_.size(); i++) {
				auto& data = mapDataList_[i];
				bool selected = (currentMapDataIndex_ == i);

				if (ImGui::Selectable(data.dataName.c_str(), selected)) {
					currentMapDataIndex_ = i;
					// 同樣不改 selectModeForLeftData_
					// 因為 Tag 決定下方視窗顯示 MapMaker
				}
			}
			ImGui::EndCombo();
		}
	}

	w2Pos = GetWindowPosLocal();
	w2Size = ImGui::GetWindowSize();

	ImGui::End();


	/// =========================
	/// Tag 控制視窗（無標題）
	/// =========================

	ImVec2 wTPos;
	ImVec2 wTSize;

	SetNextWindowPosLocal(ImVec2(0, w2Pos.y + w2Size.y));   // 放在 Mode Selector 下方
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, 0));

	ImGui::Begin("##TagController",
		nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoCollapse);

	if (IsGeneratorMode()) {
		if (ImGui::BeginTabBar("LeftTagBarGen")) {
			if (ImGui::BeginTabItem("MapData")) {
				selectModeForLeftData_ = SelectModeForLeftData::MapData;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("MapList")) {
				selectModeForLeftData_ = SelectModeForLeftData::MapList;
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	} else if (ImGui::BeginTabBar("LeftTagBar")) {
		if (ImGui::BeginTabItem("MapMaker")) {
			selectModeForLeftData_ = SelectModeForLeftData::MapMaker;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("MapData")) {
			selectModeForLeftData_ = SelectModeForLeftData::MapData;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	wTPos = GetWindowPosLocal();
	wTSize = ImGui::GetWindowSize();

	ImGui::End();


	ImVec2 w4Pos;
	ImVec2 w4Size;

	//float colimenH = (io.DisplaySize.y - wTSize.y - selectorH - templateH - titleH * 2);
	float colimenH = (io.DisplaySize.y - templateH - wTPos.y - wTSize.y);

	if (selectModeForLeftData_ == SelectModeForLeftData::MapData) {
		ImGuiMapDataWindow(wTPos, &w4Pos, wTSize, &w4Size, colimenH);
	} else if (selectModeForLeftData_ == SelectModeForLeftData::MapMaker) {
		ImGuiMapMakerWindow(wTPos, &w4Pos, wTSize, &w4Size, colimenH);
	} else if (selectModeForLeftData_ == SelectModeForLeftData::MapList) {
		ImGuiMapListWindow(wTPos, &w4Pos, wTSize, &w4Size, colimenH);
	}


	// =========================
	// 4. Template Info（獨立視窗）
	// =========================

	if (isMapDataSelected_ == 0 && isMakerSelected_ == 0 && isVariableSelected_ == 0) {
		templatePosY = w4Pos.y + w4Size.y;
		templateH = io.DisplaySize.y - templatePosY;
	}
	SetNextWindowPosLocal(ImVec2(0, templatePosY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, templateH));

	ImGui::Begin("Engine Info",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse);

	ImGui::Text("Fps/s: %.2f", system_->GetFPSPerSecond());

	ImGui::End();
}


void MapMakingEditor::ImGuiMapDataWindow(ImVec2 size, ImVec2* outputSize, ImVec2 pos, ImVec2* outputPos, float colimenH) {

	float windowY = size.y + pos.y;
	ImVec2 wSize;
	ImVec2 wPos;

	float oneColimenH = colimenH / 3;

	float titleH = ImGui::GetFrameHeight();

	SetNextWindowPosLocal(ImVec2(0, windowY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, oneColimenH));

	isMapMakerWindowOpen_ = ImGui::Begin("MapData List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);


	float full2W = leftInspectorWidth_ - 20;
	float buttonSize2 = full2W / 6.0f;
	float add2W = buttonSize2;
	float dou2W = buttonSize2;
	float sav2W = buttonSize2;
	float load2W = buttonSize2;
	float del2W = buttonSize2;


	if (ImGui::Button("New", ImVec2(add2W, 20))) {
		MapData newData{};
		newData.dataName = "new_MapData";
		if (newMapDataCounter_ != 0) newData.dataName += "_" + std::to_string(newMapDataCounter_);
		newMapDataCounter_++;
		mapDataList_.push_back(newData);
		currentMapDataIndex_ = (int)mapDataList_.size() - 1;
		currentUnitIndex_ = -1;
	}

	ImGui::SameLine();

	/// Double
	if (ImGui::Button("Double", ImVec2(dou2W, 20))) {
		if (isMapDataSelected_) {
			MapData copy = mapDataList_[currentMapDataIndex_];
			copy.dataName += "_copy";
			copy.isSaved = false;
			mapDataList_.push_back(copy);
		} else {
			debugText_ += "Error: No MapData selected for doubling.\n";
		}
	}

	ImGui::SameLine();

	/// Save
	if (ImGui::Button("Save", ImVec2(sav2W, 20))) {
		if (isMapDataSelected_) {
			MapData& d = mapDataList_[currentMapDataIndex_];
			const std::string path = FileManager::SaveFileDialog(kJsonFilter, "json",
				FileManager::ToSafeFileName(d.dataName) + ".json", "Save MapData");
			if (!path.empty()) {
				const std::string baseDir = std::filesystem::path(path).parent_path().string();
				if (WriteJsonFile(path, MapDataToJson(d, baseDir))) d.isSaved = true;
				else debugText_ += "Save Error: " + path + "\n";
			}
		} else {
			debugText_ += "Error: No MapData selected for save.\n";
		}
	}

	ImGui::SameLine();

	/// Load
	if (ImGui::Button("Load", ImVec2(load2W, 20))) {
		const std::string path = FileManager::OpenFileDialog(kJsonFilter, "Load MapData");
		nlohmann::json j;
		if (!path.empty()) {
			if (ReadJsonFile(path, j) && j.value("kind", std::string()) == "MapData") {
				const std::string baseDir = std::filesystem::path(path).parent_path().string();
				mapDataList_.push_back(JsonToMapData(j, baseDir));
				currentMapDataIndex_ = (int)mapDataList_.size() - 1;
				currentUnitIndex_ = -1;
				selectMode_ = SelectModeForMME::MapData;
			} else {
				debugText_ += "Load Error: not a MapData file: " + path + "\n";
			}
		}
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));
	if (ImGui::Button("Delete", ImVec2(del2W, 20))) {
		doubleCheck_ = true;
		selectMode_ = SelectModeForMME::MapData;
	}
	ImGui::PopStyleColor(3);

	ImGui::BeginChild("MapDataList", ImVec2(0, ImGui::GetContentRegionAvail().y), true);
	for (int i = 0; i < (int)mapDataList_.size(); i++) {
		auto& data = mapDataList_[i];
		std::string label = data.dataName + (data.isSaved ? "" : " *");

		ImGui::PushID(i);
		if (ImGui::Selectable(label.c_str(), currentMapDataIndex_ == i)) {
			if (currentMapDataIndex_ != i) currentUnitIndex_ = -1;   // 別の MapData に移ったら物件選択を解除
			currentMapDataIndex_ = i;
			selectMode_ = SelectModeForMME::MapData;
		}
		ImGui::PopID();
	}
	ImGui::EndChild();

	/// 右側パネルの表示切り替え用
	isMapDataSelected_ = (currentMapDataIndex_ >= 0 && currentMapDataIndex_ < (int)mapDataList_.size());
	isMakerSelected_ = false;

	wPos = GetWindowPosLocal();
	wSize = ImGui::GetWindowSize();

	ImGui::End();


	// =========================
	// 4. Variable List（獨立視窗）
	// =========================

	float window2Y = wPos.y + wSize.y;

	SetNextWindowPosLocal(ImVec2(0, window2Y));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, oneColimenH * 2));

	isVariablesWindowOpen_ = ImGui::Begin("Object List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float full3W = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float buttonSize3 = full3W / 6.0f;
	float add3W = buttonSize3;
	float dou3W = buttonSize3;
	float sav3W = buttonSize3;
	float load3W = buttonSize3;
	float del3W = buttonSize3;


	MapData* data = isMapDataSelected_ ? &mapDataList_[currentMapDataIndex_] : nullptr;
	const bool hasUnit = data && currentUnitIndex_ >= 0 && currentUnitIndex_ < (int)data->units.size();

	/// New
	if (ImGui::Button("New", ImVec2(add3W, 20))) {
		if (data) {
			MapUnit unit{};
			unit.id = data->NextUnitId();
			unit.name = "NewUnit" + std::to_string(unit.id);
			data->units.push_back(unit);
			currentUnitIndex_ = (int)data->units.size() - 1;
			data->isSaved = false;
		} else {
			debugText_ += "Error: No MapData selected for adding unit.\n";
		}
	}

	ImGui::SameLine();

	/// Double
	if (ImGui::Button("Double", ImVec2(dou3W, 20))) {
		if (hasUnit) {
			MapUnit copy = data->units[currentUnitIndex_];
			copy.id = data->NextUnitId();        // 代號は重複させない
			copy.name += "_copy";
			data->units.push_back(copy);
			data->isSaved = false;
		} else {
			debugText_ += "Error: No unit selected for doubling.\n";
		}
	}

	ImGui::SameLine();

	/// Save
	if (ImGui::Button("Save", ImVec2(sav3W, 20))) {
		if (hasUnit) {
			const MapUnit& u = data->units[currentUnitIndex_];
			const std::string path = FileManager::SaveFileDialog(kJsonFilter, "json",
				FileManager::ToSafeFileName(u.name) + ".json", "Save Object");
			if (!path.empty()) {
				nlohmann::json j = UnitToJson(u, std::filesystem::path(path).parent_path().string());
				j["kind"] = "MapObject";
				j["version"] = kFileVersion;
				if (!WriteJsonFile(path, j)) debugText_ += "Save Error: " + path + "\n";
			}
		} else {
			debugText_ += "Error: No object selected for save.\n";
		}
	}

	ImGui::SameLine();

	/// Load（選択中の MapData に追加、代號は振り直す）
	if (ImGui::Button("Load", ImVec2(load3W, 20))) {
		if (data) {
			const std::string path = FileManager::OpenFileDialog(kJsonFilter, "Load Object");
			nlohmann::json j;
			if (!path.empty()) {
				if (ReadJsonFile(path, j) && j.value("kind", std::string()) == "MapObject") {
					const std::string baseDir = std::filesystem::path(path).parent_path().string();
					MapUnit unit = JsonToUnit(j, baseDir);
					unit.id = data->NextUnitId();
					data->units.push_back(unit);
					currentUnitIndex_ = (int)data->units.size() - 1;
					data->isSaved = false;
				} else {
					debugText_ += "Load Error: not an Object file: " + path + "\n";
				}
			}
		} else {
			debugText_ += "Error: No MapData selected for loading object.\n";
		}
	}
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));
	if (ImGui::Button("Delete", ImVec2(del3W, 20))) {
		if (currentUnitIndex_ == kBlankUnitIndex) {
			debugText_ += "Blank cannot be deleted.\n";
		} else {
			doubleCheck_ = true;
			selectMode_ = SelectModeForMME::MapUnit;
		}
	}
	ImGui::PopStyleColor(3);

	ImGui::BeginChild("UnitList", ImVec2(0, ImGui::GetContentRegionAvail().y), true);
	if (data) {
		const std::string blankLabel = "[" + std::to_string(data->blankId) + "] Blank";
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		if (ImGui::Selectable(blankLabel.c_str(), currentUnitIndex_ == kBlankUnitIndex)) {
			currentUnitIndex_ = kBlankUnitIndex;
			selectMode_ = SelectModeForMME::MapUnit;
		}
		ImGui::PopStyleColor();
		ImGui::Separator();
		for (int i = 0; i < (int)data->units.size(); i++) {
			const MapUnit& u = data->units[i];
			std::string label = "[" + std::to_string(u.id) + "] " + u.name + (u.isDirectTile ? "" : "  (Logic)");

			ImGui::PushID(i);
			if (ImGui::Selectable(label.c_str(), currentUnitIndex_ == i)) {
				currentUnitIndex_ = i;
				selectMode_ = SelectModeForMME::MapUnit;
			}
			ImGui::PopID();
		}
	} else {
		ImGui::TextDisabled("Select a MapData first.");
	}
	ImGui::EndChild();


	*outputPos = GetWindowPosLocal();
	*outputSize = ImGui::GetWindowSize();

	ImGui::End();
}

void MapMakingEditor::ImGuiMapMakerWindow(ImVec2 size, ImVec2* outputSize, ImVec2 pos, ImVec2* outputPos, float colimenH) {

	float windowY = size.y + pos.y;
	ImVec2 wSize;
	ImVec2 wPos;

	float oneColimenH = colimenH / 3;

	float titleH = ImGui::GetFrameHeight();

	SetNextWindowPosLocal(ImVec2(0, windowY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, oneColimenH));

	isMapMakerWindowOpen_ = ImGui::Begin("MapMaker List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);


	float fullW = leftInspectorWidth_ - 20;
	float buttonSize = fullW / 6.0f;
	float addW = buttonSize;
	float douW = buttonSize;
	float savW = buttonSize;
	float loadW = buttonSize;
	float delW = buttonSize;


	if (ImGui::Button("New", ImVec2(addW, 20))) {
		MapMaker newMapMakerData;
		std::string newName = "new_MapMakerData";
		if (newMapMakerDataCounter_ != 0) newName += "_" + std::to_string(newMapMakerDataCounter_);
		newMapMakerData.SetMapName(newName);
		newMapMakerDataCounter_++;
		mapMakerlist_.push_back(newMapMakerData);
	}

	ImGui::SameLine();

	/// Double
	if (ImGui::Button("Double", ImVec2(douW, 20))) {
		if (isMakerSelected_) {
			MapMaker newMapMakerData = mapMakerlist_[currentMakerIndex_];
			newMapMakerData.SetMapName(newMapMakerData.GetMapName() + "_copy");
			mapMakerlist_.push_back(newMapMakerData);
		} else {
			debugText_ += "Error: No generator selected for doubling.\n";
		}
	}

	ImGui::SameLine();

	/// Save
	if (ImGui::Button("Save", ImVec2(savW, 20))) {
		if (isMakerSelected_) {
			auto& maker = mapMakerlist_[currentMakerIndex_];
			const std::string path = FileManager::SaveFileDialog(kJsonFilter, "json",
				FileManager::ToSafeFileName(maker.GetMapName()) + ".json", "Save MapMaker");

			if (!path.empty()) {
				const std::filesystem::path p(path);
				maker.SetMapName(p.stem().string());     // ExportMapMakerToJson は「フォルダ/mapName_.json」で書くので合わせる

				maker.SetCodingText(codeingText_);
				maker.LoadScript(codeingText_);
				maker.Compile();
				maker.BuildJson();

				if (maker.ExportMapMakerToJson(p.parent_path().string())) {
					maker.SetSaved(true);
					debugText_ += "Save: " + path + "\n";
				} else {
					debugText_ += "Save Error: Cannot write " + path + "\n";
					debugText_ += maker.GetDebugText();
				}
			}
		} else {
			debugText_ += "Error: No generator selected for save.\n";
		}
	}

	ImGui::SameLine();

	/// Load
	if (ImGui::Button("Load", ImVec2(loadW, 20))) {
		const std::string path = FileManager::OpenFileDialog(kJsonFilter, "Load MapMaker");
		if (!path.empty()) {
			MapMaker maker;
			if (maker.ImportMapMakerFromJson(path)) {
				if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {
					mapMakerlist_[currentMakerIndex_].SetCodingText(codeingText_);   // 編集中の腳本を失わないように
				}
				mapMakerlist_.push_back(maker);
				currentMakerIndex_ = (int)mapMakerlist_.size() - 1;
				selectMode_ = SelectModeForMME::MapMaker;
				codeingText_ = mapMakerlist_.back().GetCodingText();
				MapInfoLoadJson();
			}
			if (!maker.GetDebugText().empty()) debugText_ += maker.GetDebugText() + "\n";
		}
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(delW, 20))) {
		doubleCheck_ = true;
		selectMode_ = SelectModeForMME::MapMaker;
	}

	ImGui::PopStyleColor(3);

	ImGui::BeginChild("GeneratorList", ImVec2(0, ImGui::GetContentRegionAvail().y), true);
	for (int i = 0; i < mapMakerlist_.size(); i++) {
		auto& Data = mapMakerlist_[i]; // Assuming each MapMakerData has at least one prototype
		std::string label = Data.GetMapName() + (Data.isSaved() ? "" : " *");

		if (ImGui::Selectable(label.c_str(), currentMakerIndex_ == i)) {

			if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {
				mapMakerlist_[currentMakerIndex_].SetCodingText(codeingText_);
			}

			currentMakerIndex_ = i;
			selectMode_ = SelectModeForMME::MapMaker;

			// 今回選択されたGeneratorのCodingTextを取得
			codeingText_ = mapMakerlist_[i].GetCodingText();
		}
	}
	ImGui::EndChild();

	if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) isMakerSelected_ = true;
	else isMakerSelected_ = false;
	isMapDataSelected_ = false;

	wPos = GetWindowPosLocal();
	wSize = ImGui::GetWindowSize();

	ImGui::End();


	// =========================
	// Variable List
	// =========================

	float window2Y = wPos.y + wSize.y;

	SetNextWindowPosLocal(ImVec2(0, window2Y));
	if (isMapMakerWindowOpen_) {
		ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, oneColimenH * 2));
	} else {
		ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, oneColimenH * 3));
	}

	isVariablesWindowOpen_ = ImGui::Begin("Variables List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float full2W = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float buttonSize2 = full2W / 6.0f;
	float add2W = buttonSize2;
	float dou2W = buttonSize2;
	float sav2W = buttonSize2;
	float load2W = buttonSize2;
	float del2W = buttonSize2;


	if (ImGui::Button("New", ImVec2(add2W, 20))) {
		if (isMakerSelected_) {
			showNewVariablesWindow_ = true;
		} else {
			debugText_ += "Error: No generator selected for adding new variable.\n";
		}
	}

	ImGui::SameLine();

	/// Double (Duplicate Variable)
	if (ImGui::Button("Double", ImVec2(dou2W, 20))) {

		if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {

			auto& list = mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList;

			if (currentVariableIndex_ >= 0 && currentVariableIndex_ < list.size()) {

				Variable newVar = list[currentVariableIndex_];
				newVar.name += "_copy";

				list.push_back(newVar);

				debugText_ += "Variable duplicated.\n";

			} else {
				debugText_ += "Error: No variable selected for duplication.\n";
			}

		} else {
			debugText_ += "Error: No MapMaker selected for variable duplication.\n";
		}
	}

	ImGui::SameLine();

	/// Save (Apply changes to variable)
	if (ImGui::Button("Save", ImVec2(sav2W, 20))) {

		if (isMakerSelected_) {
			auto& maker = mapMakerlist_[currentMakerIndex_];
			const std::string path = FileManager::SaveFileDialog(kJsonFilter, "json",
				FileManager::ToSafeFileName(maker.GetMapName()) + ".json", "Save MapMaker (with variables)");
			if (!path.empty()) {
				const std::filesystem::path p(path);
				maker.SetMapName(p.stem().string());
				maker.SetCodingText(codeingText_);
				maker.LoadScript(codeingText_);
				maker.Compile();
				maker.BuildJson();
				if (maker.ExportMapMakerToJson(p.parent_path().string())) maker.SetSaved(true);
				else debugText_ += maker.GetDebugText() + "\n";
			}
		} else {
			debugText_ += "Error: No generator selected for save.\n";
		}

	}

	ImGui::SameLine();

	/// Load（別の MapMaker ファイルから変数を取り込む）
	if (ImGui::Button("Load", ImVec2(load2W, 20))) {
		if (isMakerSelected_) {
			const std::string path = FileManager::OpenFileDialog(kJsonFilter, "Load Variables from MapMaker");
			MapMaker tmp;
			if (!path.empty() && tmp.ImportMapMakerFromJson(path)) {
				auto& vars = mapMakerlist_[currentMakerIndex_].GetGeneratorVariables();
				for (const auto& v : tmp.GetGeneratorVariables().variableList) {
					if (Variable* exist = vars.FindVariable(v.name)) *exist = v;   // 同名は上書き
					else vars.variableList.push_back(v);
				}
				mapMakerlist_[currentMakerIndex_].SetSaved(false);
				debugText_ += "Variables loaded: " + path + "\n";
			}
		} else {
			debugText_ += "Error: No generator selected for loading variables.\n";
		}
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(del2W, 20))) {
		doubleCheck_ = true;
		selectMode_ = SelectModeForMME::Variable;
	}

	ImGui::PopStyleColor(3);

	bool skipVariableList = false;

	ImGui::BeginChild("VariableList", ImVec2(0, ImGui::GetContentRegionAvail().y), true);

	// =========================
	// MapMaker 安全檢查
	// =========================
	if (mapMakerlist_.empty()) {
		ImGui::Text("No MapMaker available.");
		skipVariableList = true;
	}

	if (!skipVariableList) {

		// =========================
		// currentMakerIndex_ 安全管理
		// =========================
		if (currentMakerIndex_ < 0) currentMakerIndex_ = 0;
		if (currentMakerIndex_ >= mapMakerlist_.size())
			currentMakerIndex_ = (int)mapMakerlist_.size() - 1;

		auto& list = mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList;

		// =========================
		// 顯示變數列表
		// =========================
		for (int i = 0; i < list.size(); i++) {

			auto& v = list[i];
			ImGui::PushID(i);

			// -------------------------
			// Type（左側）
			// -------------------------
			ImGui::Text("Type: %s", VarTypeToString(v.type).c_str());

			// -------------------------
			// 選擇用 checkbox（右側）
			// -------------------------
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20); // 靠右
			bool selected = (currentVariableIndex_ == i);
			if (ImGui::Checkbox("##select", &selected)) {
				currentVariableIndex_ = selected ? i : -1;
			}

			// -------------------------
			// Name
			// -------------------------
			InputTextString("Name", v.name);

			// -------------------------
			// Value
			// -------------------------
			switch (v.type) {
			case VarType::Bool:
				if (auto val = std::get_if<bool>(&v.value))
					InputCheckbox("Value", *val);
				break;

			case VarType::Int:
				if (auto val = std::get_if<int>(&v.value))
					InputInt("Value", *val);
				break;

			case VarType::Float:
				if (auto val = std::get_if<float>(&v.value))
					InputFloat("Value", *val);
				break;

			case VarType::Double:
				if (auto val = std::get_if<double>(&v.value))
					InputDouble("Value", *val);
				break;

			case VarType::String:
				if (auto val = std::get_if<std::string>(&v.value))
					InputTextString("Value", *val);
				break;

			case VarType::Vector2:
				if (auto val = std::get_if<Vector2>(&v.value))
					InputVector2("Value", *val);
				break;

			case VarType::Vector3:
				if (auto val = std::get_if<Vector3>(&v.value))
					InputVector3("Value", *val);
				break;

			case VarType::Vector4:
				if (auto val = std::get_if<Vector4>(&v.value))
					InputVector4("Value", *val);
				break;
			}

			ImGui::Separator();
			ImGui::PopID();
		}
	}

	ImGui::EndChild();

	*outputPos = GetWindowPosLocal();
	*outputSize = ImGui::GetWindowSize();

	ImGui::End();
}


void MapMakingEditor::ImGuiRightMenuBar() {
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 w1Pos{};
	ImVec2 w1Size{};

	float selectorH = menuBarHeight_;
	float lineH = ImGui::GetTextLineHeightWithSpacing();
	float titleH = ImGui::GetFrameHeight();

	float lastLineNum = 5;
	float debugLogH = lineH * lastLineNum + 20.0f + titleH;

	float debugLogY = io.DisplaySize.y - debugLogH;

	// ===== Camera =====
	SetNextWindowPosLocal(ImVec2(io.DisplaySize.x - rightInspectorWidth_, menuBarHeight_));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
	ImGui::Begin("Camera",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	rightInspectorWidth_ = ImGui::GetWindowSize().x;

	if (ImGui::Button("BackTo(0,0)")) {
		//debugCamera_.lock()->SetCamera(debugCamera_.lock()->GetDefaultTransform());
	}

	//ImGui::Checkbox("Use Debug Camera", &useDebugCamera);

	w1Pos = GetWindowPosLocal();
	w1Size = ImGui::GetWindowSize();

	ImGui::End();

	// ===== Info =====
	float window2Y = w1Pos.y + w1Size.y;
	ImVec2 w2Pos;
	ImVec2 w2Size;

	SetNextWindowPosLocal(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window2Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));

	const bool showMapDetail = IsGeneratorMode() && selectModeForLeftData_ == SelectModeForLeftData::MapList;
	const char* detailTitle = showMapDetail ? "Map Detail###Details"
		: isMapDataSelected_ ? "Object Detail###Details" : "Variable Details###Details";

	if (isMapDataSelected_) {
		ImGui::Begin("MapData Info",
			nullptr,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_AlwaysAutoResize);

		if (currentMapDataIndex_ >= 0 && currentMapDataIndex_ < (int)mapDataList_.size()) {
			MapData& data = mapDataList_[currentMapDataIndex_];
			ImGuiMapDataShowing(data);
		}

		w2Pos = GetWindowPosLocal();
		w2Size = ImGui::GetWindowSize();
	} else if (isMakerSelected_) {

		ImGui::Begin("Map Maker Info",
			nullptr,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_AlwaysAutoResize);

		if (currentMakerIndex_ >= 0 && currentMakerIndex_ < (int)mapMakerlist_.size()) {
			MapMaker& selectedMakerInfo = mapMakerlist_[currentMakerIndex_];
			ImGuiMapMakerShowing(selectedMakerInfo);
		}

		w2Pos = GetWindowPosLocal();
		w2Size = ImGui::GetWindowSize();

	} else {
		ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
		ImGui::Begin("Info",
			nullptr,
			ImGuiWindowFlags_NoMove);
	}

	w2Pos = GetWindowPosLocal();
	w2Size = ImGui::GetWindowSize();

	ImGui::End();

	// ===== Object Details =====
	float window3Y = w2Pos.y + w2Size.y;
	ImVec2 w3Pos;
	ImVec2 w3Size;
	w3Size.y = io.DisplaySize.y - window3Y - debugLogH - titleH;
	const float detailTargetH = w3Size.y;

	SetNextWindowPosLocal(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window3Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, w3Size.y));

	/// "表示名###ID"：### 以降が ID。タブに合わせてタイトルだけ切り替える
	const char* detailTitle2 = isMapDataSelected_ ? "Object Detail###Details" : "Variable Details###Details";
	ImGui::Begin(detailTitle2,
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	w3Pos = GetWindowPosLocal();
	w3Size = ImGui::GetWindowSize();

	if (isMakerSelected_) {
		auto& dataBase = mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList;

		for (int i = 0; i < dataBase.size(); i++) {

			Variable* v = &dataBase[i];

			ImGui::SetNextItemWidth(120.0f);
			ImGui::InputText(("##Name%s" + std::to_string(i)).c_str(), &v->name);

			ImGui::SameLine();

			ImGui::SetNextItemWidth(150.0f);
			switch (v->type) {
			case VarType::Bool:
				ImGui::Checkbox(("##Bool%s" + std::to_string(i)).c_str(), &GetValueRef<bool>(*v));
				break;
			case VarType::Int:
				ImGui::InputInt(("##Int%s" + std::to_string(i)).c_str(), &GetValueRef<int>(*v));
				break;
			case VarType::Float:
				ImGui::InputFloat(("##Float%s" + std::to_string(i)).c_str(), &GetValueRef<float>(*v));
				break;
			case VarType::Double:
				ImGui::InputDouble(("##Double%s" + std::to_string(i)).c_str(), &GetValueRef<double>(*v));
				break;
			case VarType::String:
				ImGui::InputText(("##String%s" + std::to_string(i)).c_str(), &GetValueRef<std::string>(*v));
				break;
			case VarType::Vector2:
				ImGui::InputFloat2(("##Vector2%s" + std::to_string(i)).c_str(), &GetValueRef<Vector2>(*v).x);
				break;
			case VarType::Vector3:
				ImGui::InputFloat3(("##Vector3%s" + std::to_string(i)).c_str(), &GetValueRef<Vector3>(*v).x);
				break;
			case VarType::Vector4:
				ImGui::InputFloat4(("##Vector4%s" + std::to_string(i)).c_str(), &GetValueRef<Vector4>(*v).x);
				break;
			}
		}
	} else if (isMapDataSelected_) {
		ImGuiMapUnitDetail(mapDataList_[currentMapDataIndex_], detailTargetH);
	} else if (showMapDetail && currentMapIndex_ >= 0 && currentMapIndex_ < (int)mapList_.size()) {
		ImGuiMapDetail(mapList_[currentMapIndex_]);
	}

	ImGui::End();


	// ===== Inspector Details =====
	float window4Y = w3Pos.y + w3Size.y;
	float mainBottomY = io.DisplaySize.y;
	float secondHeight = mainBottomY - window4Y;
	if (secondHeight < 0) secondHeight = 0;

	SetNextWindowPosLocal(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window4Y));
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

	ImGui::Text("%s", debugText_.c_str());

	ImGui::End();
}

void MapMakingEditor::ImGuiMidWindow() {
	ImGuiIO& io = ImGui::GetIO();

	float lineH = ImGui::GetTextLineHeightWithSpacing();
	float titleH = ImGui::GetFrameHeight();;

	// 中間視窗寬度
	float centerWidth = io.DisplaySize.x - leftInspectorWidth_ - rightInspectorWidth_;
	if (centerWidth < 0) centerWidth = 0;

	float centerPosX = leftInspectorWidth_;
	float centerPosY = menuBarHeight_;

	const bool generator = IsGeneratorMode() && selectModeForLeftData_ == SelectModeForLeftData::MapList;
	const float toolbarH = ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y * 2.0f;

	// 下のウィンドウの高さ
	int lineCount = 15; // 下のウィンドウに表示する
	float bottomH;
	float bottomWindowY;
	if (generator) {
		bottomH = toolbarH;
		bottomWindowY = io.DisplaySize.y - toolbarH;
	} else if (isMTWindowOpen_) {
		bottomH = lineH * lineCount + titleH;
		if (isMBWindowOpen_) bottomWindowY = io.DisplaySize.y - lineH * lineCount - titleH;
		else bottomWindowY = io.DisplaySize.y - titleH;
	} else {
		bottomH = io.DisplaySize.y - menuBarHeight_ - titleH;
		bottomWindowY = menuBarHeight_ + titleH;
	}

	// 中間視窗高度
	float centerTotalH;
	if (generator)          centerTotalH = io.DisplaySize.y - menuBarHeight_ - toolbarH;
	else if (isMBWindowOpen_) centerTotalH = io.DisplaySize.y - menuBarHeight_ - bottomH;
	else                    centerTotalH = io.DisplaySize.y - menuBarHeight_ - menuBarHeight_;

	// ===== Top Window（跟 Maps / Template Info 一樣的寫法）=====
	SetNextWindowPosLocal(ImVec2(centerPosX, centerPosY));
	ImGui::SetNextWindowSize(ImVec2(centerWidth, centerTotalH));

	const bool showMapPreview = generator && selectModeForLeftData_ == SelectModeForLeftData::MapList;
	const bool showPreview = !showMapPreview && selectModeForLeftData_ == SelectModeForLeftData::MapData;

	/// タイトルだけ切り替え（### 以降が ID。位置・サイズの計算はそのまま使える）
	const char* midTitle = generator ? "Map Preview###MidTop"
		: showPreview ? "Object Preview###MidTop" : "Map###MidTop";
	ImGuiWindowFlags midFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
	if (showMapPreview || showPreview) midFlags |= ImGuiWindowFlags_NoScrollWithMouse;

	isMTWindowOpen_ = ImGui::Begin(midTitle, nullptr, midFlags);

	if (showMapPreview) {

		ImGuiMapPreview();

	} else if (showPreview) {
		MapData* data = isMapDataSelected_ ? &mapDataList_[currentMapDataIndex_] : nullptr;

		ImGuiObjectPreview(data);

	} else if (ImGui::BeginTabBar("##MapTabs")) {

		if (ImGui::BeginTabItem("Map")) {
			if (!currentMapInfo_.tileMapData.Row.empty()) {
				ImGui::Text("%s", currentMapInfo_.tileMapData.TileMapDataToString().c_str());
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Compile")) {
			ImGui::Text("Compile Result:");

			ImGui::Text("%s", currentMapInfo_.jsonData.c_str());

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();


	// ===== Bottom Window =====
	SetNextWindowPosLocal(ImVec2(centerPosX, bottomWindowY));
	ImGui::SetNextWindowSize(ImVec2(centerWidth, bottomH));

	ImGuiWindowFlags bottomFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
	if (generator) bottomFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;

	isMBWindowOpen_ = ImGui::Begin("Inspector Details", nullptr, bottomFlags);

	if (generator) {
		ImGuiMapToolbar();
	} else {
		InputBigTextString("Codeing Text", codeingText_);

		if (ImGui::Button("Save")) {
			if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {

				auto& maker = mapMakerlist_[currentMakerIndex_];
				maker.SetCodingText(codeingText_);
				maker.LoadScript(codeingText_);

				maker.Compile();
				maker.BuildJson();
				maker.SetSaved(false);
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Apply")) {
			if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {

				auto& maker = mapMakerlist_[currentMakerIndex_];
				maker.SetCodingText(codeingText_);
				maker.LoadScript(codeingText_);

				maker.Compile();
				maker.BuildJson();
				MapInfoLoadJson();
				maker.SetSaved(false);

				debugText_ += "Apply: Script updated and recompiled.\n";
			} else {
				debugText_ += "Error: No valid map maker selected.\n";
			}
		}

		ImGui::SameLine();
		ImGui::Text("Result = result[y][x]");
	}

	ImGui::End();
}

void MapMakingEditor::ImGuiLoadWindow() {
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


void MapMakingEditor::ImGuiSaveWindow() {
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
			selectMode_ = SelectModeForMME::None;
		}
		if (isButtonClicked) {
			showSaveWindow_ = false;
		}
		ImGui::End();
	}
}

void MapMakingEditor::ImGuiDoubleCheckWindow() {
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
			selectMode_ = SelectModeForMME::None;
		}
		if (isButtonClicked) {
			doubleCheck_ = false;
		}
		ImGui::End();
	}
}

void MapMakingEditor::ImGuiMapNewWindow() {

	/// 最後の時だけOpenPopupをする、ついでに名前を初期化する
	if (showNewWindow_) {
		ImGui::OpenPopup("New Map");

		std::string newNumber = std::to_string(newVariableCounter_);
		if (newVariableCounter_ == 0)newNumber = "";
		newName_ = "NewMap" + newNumber;

		showNewWindow_ = false;
	}

	/// ウィンドウのサイズを計算、固定する
	float lineHeight = ImGui::GetTextLineHeightWithSpacing();
	float padding = ImGui::GetStyle().WindowPadding.y * 2;
	float titleBar = ImGui::GetFrameHeight();
	float fixedHeight = lineHeight * 5 + padding + titleBar;
	ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));

	if (ImGui::BeginPopupModal("New Map", NULL, ImGuiWindowFlags_NoResize)) {

		/// セーブパース
		InputTextString("Save File Path", saveFilePath_);
		/// 名前
		InputTextString("Map Name", newName_);

		/// サイズ

		/// サイズ
		ImGui::Text("Size(x, y):");
		ImGui::SameLine();
		ImGui::InputInt2("##Size", &newMapSize_[0]);

		bool isButtonClicked = false;
		if (ImGui::Button("New")) {

			// Add Map 動作
			MapInfo newMap{};

			newMap.tileMapData = TileMapData{};
			newMap.tileMapData.name = newName_;
			for (int y = 0; y < newMapSize_[1]; ++y) {
				for (int x = 0; x < newMapSize_[0]; ++x) {
					newMap.tileMapData.Column.push_back(0);
				}
				newMap.tileMapData.ColumnPush();
			}

			newMap.isSaved = false;
			newMap.savePath = saveFilePath_;
			mapList_.push_back(newMap);


			std::string newNumber = std::to_string(newVariableCounter_);
			if (newVariableCounter_ == 0)newNumber = "";
			if (newName_ == "NewMap" + newNumber) {
				newMapCounter_++;
			}

			isButtonClicked = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
		}
		if (isButtonClicked) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}


void MapMakingEditor::ImGuiMapUnitDetail(MapData& data, float targetHeight) {

	if (data.units.empty()) {
		ImGui::TextDisabled("No unit. Press New in Variables List.");
		return;
	}

	/// 番号 + 代號 + 名前でリンク（Variables List と同じ currentUnitIndex_）
	auto makeLabel = [](int i, const MapUnit& u) {
		return "#" + std::to_string(i) + "  [" + std::to_string(u.id) + "] " + u.name;
		};
	const bool valid = currentUnitIndex_ >= 0 && currentUnitIndex_ < (int)data.units.size();
	std::string preview = valid ? makeLabel(currentUnitIndex_, data.units[currentUnitIndex_])
		: (currentUnitIndex_ == kBlankUnitIndex ? "[" + std::to_string(data.blankId) + "] Blank" : "(select unit)");

	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::BeginCombo("##UnitSelect", preview.c_str())) {
		if (ImGui::Selectable(("[" + std::to_string(data.blankId) + "] Blank").c_str(), currentUnitIndex_ == kBlankUnitIndex)) {
			currentUnitIndex_ = kBlankUnitIndex;
		}
		for (int i = 0; i < (int)data.units.size(); ++i) {
			const bool selected = (i == currentUnitIndex_);
			ImGui::PushID(i);
			if (ImGui::Selectable(makeLabel(i, data.units[i]).c_str(), selected)) currentUnitIndex_ = i;
			if (selected) ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}
	if (currentUnitIndex_ == kBlankUnitIndex) {
		int idEdit = data.blankId;
		ImGui::InputInt("ID", &idEdit, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
		if (ImGui::IsItemDeactivatedAfterEdit() && idEdit != data.blankId) ChangeUnitId(data, data.blankId, idEdit);
		ImGui::SetItemTooltip("Enter で確定。既存の番号なら入れ替え");
		ImGui::TextDisabled("空きマス（モデル無し・削除不可）");
		return;
	}
	if (!valid) return;

	/// ================= 中身は固定サイズの子ウィンドウ（全体の排版を保つため） ================= ///
	const ImGuiStyle& style = ImGui::GetStyle();
	float bodyH = targetHeight
		- ImGui::GetFrameHeight()              // タイトルバー
		- style.WindowPadding.y * 2.0f         // 上下の余白
		- ImGui::GetFrameHeightWithSpacing();  // 上の Combo の行
	if (bodyH < 60.0f) bodyH = 60.0f;
	const float bodyW = rightInspectorWidth_ - style.WindowPadding.x * 2.0f;

	ImGui::BeginChild("##ObjectDetailBody", ImVec2(bodyW, bodyH), false);

	MapUnit& unit = data.units[currentUnitIndex_];
	bool changed = false;
	ImGui::PushID(currentUnitIndex_);

	/// ---------------- Basic ---------------- ///
	if (ImGui::CollapsingHeader("Basic", ImGuiTreeNodeFlags_DefaultOpen)) {
		int idEdit = unit.id;
		ImGui::InputInt("ID", &idEdit, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue);
		if (ImGui::IsItemDeactivatedAfterEdit() && idEdit != unit.id) ChangeUnitId(data, unit.id, idEdit);
		ImGui::SetItemTooltip("Enter で確定。既存の番号なら入れ替え");

		changed |= InputTextString("Name", unit.name);
		changed |= ImGui::Checkbox("Direct Tile", &unit.isDirectTile);
		ImGui::SetItemTooltip("ON : 石塊・磚塊のように置くだけ\nOFF: 特殊ロジックあり");
		changed |= ImGui::ColorEdit4("Preview Color", &unit.previewColor.x);

		/// ---- Model（handle だけ持ち、パスはエンジンに問い合わせる） ----
		auto modelTexture = [this, &unit](int handle) -> int {
			/// ファイルのモデルだけが自分の texture を持っている（内蔵モデルは -1 なので使えない）
			if (unit.engineModel == EngineModelType::None && handle >= 0 && system_->GetMutiModelNum(handle) > 0) {
				return system_->GetModelTextureHandle(handle, 0);
			}
			return config::default_White5x5_TextureHandle_;
			};

		/// 内蔵モデルなら "[Engine] Sphere" のように表示
		const std::string modelLabel = (unit.engineModel != EngineModelType::None)
			? "[Engine] " + GetEnumName(unit.engineModel)
			: GetModelPath(unit.modelHandle);

		std::string newPath;
		switch (ImGuiWidgets::FileSlot("Model", modelLabel, kModelFilter, newPath, "Engine")) {
		case ImGuiWidgets::FileSlotAction::Load: {
			const int handle = system_->SetModelObj(newPath);
			if (system_->GetMutiModelNum(handle) == 0) {
				debugText_ += "Model Error: cannot load " + newPath + "\n";
				break;
			}
			unit.modelHandle = handle;
			unit.engineModel = EngineModelType::None;          // ファイルのモデルに戻る
			if (unit.material.useModelTexture) unit.material.textureHandle = modelTexture(handle);
			changed = true;
			break;
		}
		case ImGuiWidgets::FileSlotAction::Extra:              // 「Engine」ボタン → 選択ウィンドウを開く
			showEngineModelWindow_ = true;
			engineTargetMapData_ = currentMapDataIndex_;
			engineTargetUnit_ = currentUnitIndex_;
			if (unit.engineModel != EngineModelType::None) {    // 内蔵モデルなら今の設定から始める
				engineSelectType_ = unit.engineModel;
				engineEditParams_ = enginePreviewParams_ = unit.engineParams;
			}
			break;
		case ImGuiWidgets::FileSlotAction::Delete:
			unit.modelHandle = -1;
			unit.engineModel = EngineModelType::None;
			if (unit.material.useModelTexture) unit.material.textureHandle = modelTexture(-1);
			changed = true;
			break;
		default: break;
		}

		/// ---- Texture（自分で選んだ時だけファイル名を出す） ----
		const std::string texPath = unit.material.useModelTexture
			? ""
			: system_->GetTextureManager()->GetTextureFilePath(unit.material.textureHandle);

		switch (ImGuiWidgets::FileSlot("Texture", texPath, kTextureFilter, newPath, "Engine")) {
		case ImGuiWidgets::FileSlotAction::Load:
			unit.material.textureHandle = system_->LoadTexture(newPath);
			unit.material.useModelTexture = false;
			changed = true;
			break;
		case ImGuiWidgets::FileSlotAction::Delete:
			unit.material.useModelTexture = true;
			unit.material.textureHandle = modelTexture(unit.modelHandle);
			changed = true;
			break;
		case ImGuiWidgets::FileSlotAction::Extra:
			debugText_ += "Engine Texture: not implemented yet.\n";    // 将来ここで内蔵 texture 選択を開く
			break;
		default: break;
		}
	}

	/// ---------------- Transform ---------------- ///
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		changed |= ImGuiWidgets::TransformEditor(unit.transform);
	}

	/// ---------------- Material（Texture は上で出したので隠す） ---------------- ///
	if (ImGui::CollapsingHeader("Material")) {
		changed |= ImGuiWidgets::MaterialConfigEditor(unit.material, true, false);
	}

	ImGui::PopID();
	ImGui::EndChild();       // PushID → BeginChild の逆順で閉じる

	if (changed) data.isSaved = false;
}

void MapMakingEditor::ImGuiMapDataShowing(MapData& data) {

	ImGui::Text("Selected Map: %s", data.dataName.c_str());
	if (InputTextString("Map Name", data.dataName)) data.isSaved = false;
	InputInt("Max Size X", data.xSize);
	InputInt("Max Size Y", data.ySize);
	if (ImGui::DragFloat2("Cell Spacing", &data.cellSpacing.x, 0.01f, 0.01f, 100.0f, "%.2f")) data.isSaved = false;
	ImGui::SetItemTooltip("マスの間隔（ゲーム側の BlockWidth / BlockHeight）");
	ImGui::Text("Units: %d", (int)data.units.size());

}

void MapMakingEditor::ImGuiMapMakerShowing(MapMaker& maker) {

	std::string name = maker.GetMapName();
	InputTextString("Map Name", name);
	maker.SetMapName(name);

	std::string savePath = maker.GetProjectPath();
	InputTextString("Save Path", savePath);
	maker.SetProjectPath(savePath);
	ImGui::Text("VariablesNumber:%d", static_cast<int>(maker.GetGeneratorVariables().variableList.size()));

}

void MapMakingEditor::ImGuiNewVariablesWindow() {
	/// 最後の時だけOpenPopupをする、ついでに名前を初期化する
	if (showNewVariablesWindow_) {
		ImGui::OpenPopup("New Variables");

		std::string newNumber = std::to_string(newVariableCounter_);
		if (newVariableCounter_ == 0)newNumber = "";
		newName_ = "NewVariable" + newNumber;

		showNewVariablesWindow_ = false;
	}

	/// ウィンドウのサイズを計算、固定する
	float lineHeight = ImGui::GetTextLineHeightWithSpacing();
	float padding = ImGui::GetStyle().WindowPadding.y * 2;
	float titleBar = ImGui::GetFrameHeight();
	float fixedHeight = lineHeight * 5 + padding + titleBar;
	ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));

	/// Popup Window開始
	if (ImGui::BeginPopupModal("New Variables", NULL,
		ImGuiWindowFlags_NoResize)) {


		/// 名前
		InputTextString("Variables Name", newName_);

		/// タイプ
		ImGui::Text("Type:");
		ImGui::SameLine();
		const char* types[] = {
			"Bool\0", "Int\0", "Float\0", "Double\0",
			"String\0", "Vector2\0", "Vector3\0", "Vector4\0"
		};
		ImGui::Combo("##VariableType", &variableType_, types, IM_ARRAYSIZE(types));


		switch (variableType_) {
		case (int)VarType::Bool:
			InputCheckbox("Bool Value", newBoolValue_);
			break;
		case (int)VarType::Int:
			InputInt("Int Value", newIntValue_);
			break;
		case (int)VarType::Float:
			InputFloat("Float Value", newFloatValue_);
			break;
		case (int)VarType::Double:
			InputDouble("Double Value", newDoubleValue_);
			break;
		case (int)VarType::String:
			InputTextString("String Value", newStringValue_);
			break;
		case (int)VarType::Vector2:
			InputVector2("Vector2 Value", newVector2Value_);
			break;
		case (int)VarType::Vector3:
			InputVector3("Vector3 Value", newVector3Value_);
			break;
		case (int)VarType::Vector4:
			InputVector4("Vector4 Value", newVector4Value_);
			break;
		}


		bool isButtonClicked = false;
		if (ImGui::Button("New")) {

			if (currentMakerIndex_ != -1) {
				// Add VariableUnit 動作
				Variable newVariable{};

				// 初始化 value
				switch ((VarType)variableType_) {
				case VarType::Bool: newVariable = CreateVariable(newName_, newBoolValue_);			break;
				case VarType::Int: newVariable = CreateVariable(newName_, newIntValue_);			break;
				case VarType::Float: newVariable = CreateVariable(newName_, newFloatValue_);			break;
				case VarType::Double: newVariable = CreateVariable(newName_, newDoubleValue_);		break;
				case VarType::String: newVariable = CreateVariable(newName_, newStringValue_);		break;
				case VarType::Vector2: newVariable = CreateVariable(newName_, newVector2Value_);		break;
				case VarType::Vector3: newVariable = CreateVariable(newName_, newVector3Value_);		break;
				case VarType::Vector4: newVariable = CreateVariable(newName_, newVector4Value_);		break;
				}

				/// [選択したGenerator][変数の種類]に新しい変数を追加する
				mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList.push_back(newVariable);

				std::string newNumber = std::to_string(newVariableCounter_);
				if (newVariableCounter_ == 0)newNumber = "";
				if (newName_ == "NewVariable" + newNumber) {
					newVariableCounter_++;
				}
				isButtonClicked = true;
			} else {
				/// 後でエラーウィンドウに変わる
				Logger::Log("MapMakingEditor: No generator selected. Cannot add new variable.");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
		}
		if (isButtonClicked) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
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

bool MapMakingEditor::InputTextString(const char* label, std::string& str, ImGuiInputTextFlags flags) {
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

bool MapMakingEditor::InputBigTextString(const char* label, std::string& str, ImGuiInputTextFlags flags) {
	flags |= ImGuiInputTextFlags_CallbackResize;

	ImGui::Text("%s:", label);

	std::string newLabel = std::string("##") + label; // 隐藏标签

	if (str.capacity() == 0)
		str.reserve(16);

	// 取得視窗內容區域大小（扣掉 Text 的高度）
	ImVec2 avail = ImGui::GetContentRegionAvail();

	// 行の高さを取得
	float lineHeight = ImGui::GetTextLineHeightWithSpacing();

	// 下方 padding
	avail.y -= ImGui::GetTextLineHeight();
	avail.y -= lineHeight * 2;

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

void MapMakingEditor::InputCheckbox(const char* label, bool& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::Checkbox(newLabel.c_str(), &value);
}

void MapMakingEditor::InputInt(const char* label, int& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputInt(newLabel.c_str(), &value);
}

void MapMakingEditor::InputFloat(const char* label, float& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat(newLabel.c_str(), &value);
}

void MapMakingEditor::InputDouble(const char* label, double& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputDouble(newLabel.c_str(), &value);
}

void MapMakingEditor::InputVector2(const char* label, Vector2& vec) {

	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat2(newLabel.c_str(), &vec.x);
}

void MapMakingEditor::InputVector3(const char* label, Vector3& vec) {

	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat3(newLabel.c_str(), &vec.x);
}

void MapMakingEditor::InputVector4(const char* label, Vector4& vec) {

	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat4(newLabel.c_str(), &vec.x);
}

#endif

/// ImDrawList* draw = ImGui::GetWindowDrawList();
/// ImVec2 p = ImGui::GetCursorScreenPos();
/// draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 100, p.y + 50), IM_COL32(0, 255, 0, 255));

void MapMakingEditor::ImGuiMapDetail(MapInfo& info) {
	const MapData* pd = FindMapData(info.mapDataName);
	const int blank = pd ? pd->blankId : 0;
	TileMapData& map = info.tileMapData;

	if (InputTextString("Name", map.name)) info.isSaved = false;

	/// 使う MapData（名前で紐づけ）
	const char* preview = info.mapDataName.empty() ? "(none)" : info.mapDataName.c_str();
	if (ImGui::BeginCombo("MapData", preview)) {
		for (const auto& d : mapDataList_) {
			if (ImGui::Selectable(d.dataName.c_str(), d.dataName == info.mapDataName)) {
				info.mapDataName = d.dataName;
				info.isSaved = false;
			}
		}
		ImGui::EndCombo();
	}
	if (!info.mapDataName.empty() && !FindMapData(info.mapDataName)) {
		ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "MapData not found!");
	}

	/// サイズは Apply で反映（縮めると消えるので、打つたびには変えない）
	ImGui::InputInt2("Size (W,H)", mapResizeBuf_);
	if (ImGui::Button("Apply Size")) {
		map.Resize((std::max)(1, mapResizeBuf_[0]), (std::max)(1, mapResizeBuf_[1]), blank);
		info.isSaved = false;
		mapFocusIndex_ = -2;                 // カメラを全体に合わせ直す
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")) { map.Fill(blank); info.isSaved = false; }

	ImGui::TextDisabled("File: %s", info.savePath.empty() ? "(Unsaved)" : info.savePath.c_str());
}

void MapMakingEditor::ImGuiMapPreview() {
	if (currentMapIndex_ < 0 || currentMapIndex_ >= (int)mapList_.size()) {
		ImGui::TextDisabled("Select a map from MapList.");
		return;
	}
	MapInfo& info = mapList_[currentMapIndex_];
	TileMapData& map = info.tileMapData;
	MapData* palette = FindMapData(info.mapDataName);
	const Vector2 sp = palette ? palette->cellSpacing : Vector2{ 1.0f, 1.0f };
	const int H = map.Height();

	if (currentMapIndex_ != mapFocusIndex_) {
		mapFocusIndex_ = currentMapIndex_;
		FocusMapCamera(map, sp);
	}

	/// ===== 1. 見本 Object を物件ごとに用意 =====
	std::unordered_map<int, Object*> live;                    // 今の MapData にある物件だけ（消した物件は描かない）
	if (palette) {
		for (const MapUnit& unit : palette->units) {
			auto& obj = mapTileTemplates_[unit.id];
			SyncUnitObject(obj, unit);                            // モデルが変わった時だけ作り直す
			if (obj) live[unit.id] = obj.get();
		}
	}

	/// ===== 2. マスごとに見本をコピーして位置だけずらす =====
	mapPreviewCells_.clear();
	mapPreviewPtrs_.clear();
	int instanceCount = 0;
	bool overflow = false;
	for (int y = 0; y < H && !overflow; ++y) {
		for (int x = 0; x < (int)map.Row[y].size(); ++x) {
			auto it = live.find(map.Row[y][x]);
			if (it == live.end()) continue;                       // 0（空き）/ 未定義 / モデル無し
			const int parts = (int)it->second->objectParts_.size();
			if (instanceCount + parts > kMaxPreviewInstances) { overflow = true; break; }

			ObjectData cell = *it->second;                        // Object の ObjectData 部分だけコピー
			const Vector3 c = CellToWorld(x, y, H, sp);
			Vector3& t = cell.mainPosition.transform.translate;
			t = Vector3{ t.x + c.x, t.y + c.y, t.z + c.z };       // Object Detail の Translate はオフセットとして残る
			mapPreviewCells_.push_back(std::move(cell));
			instanceCount += parts;
		}
	}
	for (auto& c : mapPreviewCells_) mapPreviewPtrs_.push_back(&c);   // push_back が全部終わってからポインタを取る

	/// ===== 3. 画像 + 入力 =====
	const ImVec2 avail = ImGui::GetContentRegionAvail();
	float w = avail.x;
	float h = w * 0.75f;
	if (h > avail.y) { h = avail.y; w = h / 0.75f; }
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail.x - w) * 0.5f);

	const ImVec2 p = ImGui::GetCursorScreenPos();
	const ImVec2 size(w, h);
	DrawEngine* de = system_->GetDrawEngine();

	ImGui::Image((ImTextureID)de->GetPreviewTextureID(), size);
	ImGui::SetCursorScreenPos(p);
	ImGui::InvisibleButton("##mapPreview", size,
		ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

	const float camSpeed = (std::max)(1.0f, (std::max)(sp.x, sp.y) * (std::max)(map.Width(), H) * 0.25f);
	if (UpdateFreeCamera(mapCamPos_, mapCamRot_, camSpeed)) FocusMapCamera(map, sp);   // ← InvisibleButton の直後
	const bool hovered = ImGui::IsItemHovered();
	const bool active = ImGui::IsItemActive();

	Matrix4x4 view = Inverse(MakeAffineMatrix(Vector3{ 1.0f, 1.0f, 1.0f }, mapCamRot_, mapCamPos_));
	Matrix4x4 proj = MakePerspectiveFovMatrix(0.7f, 512.0f / 384.0f, 0.05f, 2000.0f);
	Matrix4x4 viewProj = view * proj;
	const Matrix4x4 toScreen = MakeScreenMatrix(viewProj, p, size);

	/// ===== 4. 道具ごとの入力 =====
	ImGuiIO& io = ImGui::GetIO();
	int cx = -1, cy = -1;
	const bool onCell = (hovered || active) && ScreenToCell(io.MousePos, toScreen, map, sp, cx, cy);
	const int blank = palette ? palette->blankId : 0;

	auto apply = [&](const CellList& cells, int value) {
		bool changed = false;
		for (const auto& [x, y] : cells) {
			if (map.InBounds(x, y) && map.Row[y][x] != value) { map.Row[y][x] = value; changed = true; }
		}
		if (changed) info.isSaved = false;
		};

	/// ペン / 消しゴム：押している間ずっと
	if ((mapTool_ == MapTool::Paint || mapTool_ == MapTool::Erase) && onCell && active && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
		apply({ { cx, cy } }, mapTool_ == MapTool::Paint ? paintUnitId_ : blank);
	}

	/// 塗りつぶし：クリックした瞬間に1回だけ
	if (mapTool_ == MapTool::Fill && onCell && hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		if (FloodFill(map, cx, cy, paintUnitId_) > 0) info.isSaved = false;
	}

	/// 四角 / 円：押したマス 〜 離したマス
	const bool shapeTool = (mapTool_ == MapTool::Rect || mapTool_ == MapTool::Circle);
	if (!shapeTool) shapeDragging_ = false;
	CellList shapeCells;
	if (shapeTool) {
		if (onCell && hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			shapeDragging_ = true;
			shapeX0_ = shapeX1_ = cx;
			shapeY0_ = shapeY1_ = cy;
		}
		if (shapeDragging_) {
			/// マップの外に出ても端に寄せて追いかける（ScreenToCell は範囲外でも座標は入れてくれる）
			int ex = shapeX1_, ey = shapeY1_;
			ScreenToCell(io.MousePos, toScreen, map, sp, ex, ey);
			ex = std::clamp(ex, 0, map.Width() - 1);
			ey = std::clamp(ey, 0, H - 1);

			/// Shift：正方形 / 正円
			if (io.KeyShift) {
				const int side = (std::max)(std::abs(ex - shapeX0_), std::abs(ey - shapeY0_));
				ex = shapeX0_ + (ex >= shapeX0_ ? side : -side);
				ey = shapeY0_ + (ey >= shapeY0_ ? side : -side);
			}
			shapeX1_ = ex;
			shapeY1_ = ey;

			shapeCells = (mapTool_ == MapTool::Rect)
				? RectCells(shapeX0_, shapeY0_, shapeX1_, shapeY1_, shapeFilled_)
				: EllipseCells(shapeX0_, shapeY0_, shapeX1_, shapeY1_, shapeFilled_);

			if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {             // キャンセル
				shapeDragging_ = false;
				shapeCells.clear();
			} else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				apply(shapeCells, paintUnitId_);
				shapeDragging_ = false;
			}
		}
	}

	/// ===== 5. 重ね描き（外枠・カーソルのマス・文字） =====
	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->PushClipRect(p, ImVec2(p.x + w, p.y + h), true);

	auto project = [&](float x, float y, ImVec2& out) {
		const Vector3 s = viewFinalTransform(Vector3{ x, y, 0.0f }, toScreen);
		if (s.z < 0.0f || s.z > 1.0f) return false;
		out = ImVec2(s.x, s.y);
		return true;
		};
	auto worldRect = [&](float x0, float y0, float x1, float y1, ImU32 col, float thickness) {
		ImVec2 a, b, c, d;
		if (project(x0, y0, a) && project(x1, y0, b) && project(x1, y1, c) && project(x0, y1, d)) {
			dl->AddQuad(a, b, c, d, col, thickness);
		}
		};
	auto worldFill = [&](float x0, float y0, float x1, float y1, ImU32 col) {
		ImVec2 a, b, c, d;
		if (project(x0, y0, a) && project(x1, y0, b) && project(x1, y1, c) && project(x0, y1, d)) {
			dl->AddQuadFilled(a, b, c, d, col);
		}
		};

	worldRect(0.0f, 0.0f, map.Width() * sp.x, H * sp.y, IM_COL32(255, 255, 255, 90), 1.0f);
	/// 四角・円：離すと塗られるマスを半透明で表示
	for (const auto& [x, y] : shapeCells) {
		const Vector3 c = CellToWorld(x, y, H, sp);
		worldFill(c.x - sp.x * 0.5f, c.y - sp.y * 0.5f, c.x + sp.x * 0.5f, c.y + sp.y * 0.5f, IM_COL32(90, 200, 255, 90));
	}

	if (onCell) {
		const Vector3 c = CellToWorld(cx, cy, H, sp);
		const ImU32 col = (mapTool_ == MapTool::Erase) ? IM_COL32(255, 90, 90, 255)
			: (mapTool_ == MapTool::Paint) ? IM_COL32(90, 255, 140, 255) : IM_COL32(255, 255, 255, 160);
		worldRect(c.x - sp.x * 0.5f, c.y - sp.y * 0.5f, c.x + sp.x * 0.5f, c.y + sp.y * 0.5f, col, 2.0f);
	}

	char buf[128];
	snprintf(buf, sizeof(buf), "%s  (%d x %d)", map.name.c_str(), map.Width(), H);
	dl->AddText(ImVec2(p.x + 8.0f, p.y + 6.0f), IM_COL32(170, 180, 198, 255), buf);
	if (onCell) {
		snprintf(buf, sizeof(buf), "(%d, %d)  id = %d", cx, cy, map.Row[cy][cx]);
		dl->AddText(ImVec2(p.x + 8.0f, p.y + 24.0f), IM_COL32(170, 180, 198, 255), buf);
	}
	if (!palette) dl->AddText(ImVec2(p.x + 8.0f, p.y + 42.0f), IM_COL32(255, 120, 120, 255), "No MapData linked (Map Detail)");
	if (overflow) dl->AddText(ImVec2(p.x + 8.0f, p.y + 60.0f), IM_COL32(255, 200, 80, 255), "Too many blocks: preview truncated");
	dl->AddText(ImVec2(p.x + 8.0f, p.y + h - 20.0f), IM_COL32(120, 130, 150, 255),
		"LMB: tool (Shift: square/circle, Esc: cancel)   RMB: look   MMB: pan   Wheel: zoom   R: focus");
	dl->PopClipRect();

	/// ===== 6. 描画申請 =====
	DrawEngine::PreviewRequest req;
	req.active = true;
	req.objects = mapPreviewPtrs_;
	req.view.view = view;
	req.view.proj = proj;
	req.view.position = mapCamPos_;
	de->SetPreviewRequest(req);
}

void MapMakingEditor::ImGuiMapToolbar() {
	/// 押されている道具はボタンの色を変える。もう一度押すと解除
	auto toolButton = [this](const char* label, MapTool tool) {
		const bool on = (mapTool_ == tool);
		if (on) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (ImGui::Button(label)) mapTool_ = on ? MapTool::None : tool;
		if (on) ImGui::PopStyleColor();
		};

	toolButton("Paint", MapTool::Paint);
	ImGui::SameLine();
	toolButton("Erase", MapTool::Erase);
	ImGui::SameLine();
	toolButton("Fill", MapTool::Fill);
	ImGui::SameLine();
	toolButton("Rect", MapTool::Rect);
	ImGui::SameLine();
	toolButton("Circle", MapTool::Circle);
	ImGui::SameLine();
	ImGui::Checkbox("Filled", &shapeFilled_);
	ImGui::SetItemTooltip("四角・円：ON = 塗りつぶし / OFF = 外周だけ");
	ImGui::SameLine();
	ImGui::TextDisabled("|");
	ImGui::SameLine();

	/// 塗る物件（選択中のマップが使う MapData から）
	MapInfo* info = (currentMapIndex_ >= 0 && currentMapIndex_ < (int)mapList_.size()) ? &mapList_[currentMapIndex_] : nullptr;
	MapData* palette = info ? FindMapData(info->mapDataName) : nullptr;
	const MapUnit* cur = palette ? palette->FindUnit(paintUnitId_) : nullptr;
	const std::string preview = cur ? "[" + std::to_string(cur->id) + "] " + cur->name
		: (palette && paintUnitId_ == palette->blankId ? "[" + std::to_string(paintUnitId_) + "] Blank" : "(none)");

	ImGui::SetNextItemWidth(180.0f);
	ImGui::BeginDisabled(palette == nullptr);
	if (ImGui::BeginCombo("##PaintTile", preview.c_str())) {
		if (ImGui::Selectable(("[" + std::to_string(palette->blankId) + "] Blank").c_str(), paintUnitId_ == palette->blankId)) {
			paintUnitId_ = palette->blankId;
		}
		for (const auto& u : palette->units) {
			const std::string label = "[" + std::to_string(u.id) + "] " + u.name;
			if (ImGui::Selectable(label.c_str(), u.id == paintUnitId_)) paintUnitId_ = u.id;
		}
		ImGui::EndCombo();
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::SetNextItemWidth(80.0f);
}