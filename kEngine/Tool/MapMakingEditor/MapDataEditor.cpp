#include "MapDataEditor.h"
#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include "ImGuiManager.h"
#include "Tool/FileManager/FileManager.h"

namespace {
	constexpr int kDefaultWidth = 64;    // 1280 / 20
	constexpr int kDefaultHeight = 36;   // 720  / 20
	constexpr float kMinCell = 2.0f;
	constexpr float kMaxCell = 64.0f;

	bool ContainsNoCase(const std::string& text, const std::string& key) {
		auto it = std::search(text.begin(), text.end(), key.begin(), key.end(),
			[](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
		return it != text.end();
	}
}

/// ColorMatcher::typeColor と同じ色（0xRRGGBBAA → ImGui の ABGR）
unsigned int MapDataEditor::TileColor(int type) {
	static const unsigned int kRGBA[MAX_TYPE] = {
		0x000000FF, // NONE
		0x1A1E28FF, // AIR（背景と区別できるように少しだけ色を付ける）
		0x00BB00FF, // GRASS
		0xBB5E00FF, // EARTH
		0x6C6C6CFF, // ROCK
		0xE0E0E0FF, // IRON
		0xFFD700FF, // GOLD
		0x8080C0FF, // DIAMOND
		0xFF00FFFF, // MONSTERSPAWNER
		0x00FFFFFF, // DUNGEONDOOR
	};
	const unsigned int c = (type >= 0 && type < MAX_TYPE) ? kRGBA[type] : 0xFF0000FF;
	return IM_COL32((c >> 24) & 0xFF, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
}

MapData MapDataEditor::CreateMapData(const std::string& name, int width, int height) {
	MapData data{};
	data.dataName = name;
	data.xSize = width;
	data.ySize = height;
	data.tiles.name = name;
	data.tiles.Resize(width, height, AIR);
	data.isSaved = false;
	return data;
}

void MapDataEditor::SyncSize(MapData& data) {
	if (data.tiles.Width() != data.xSize || data.tiles.Height() != data.ySize) {
		data.tiles.Resize(data.xSize, data.ySize, AIR);
	}
}

void MapDataEditor::Draw(std::vector<MapData>& list, std::string& log) {
#ifdef USE_IMGUI
	if (selected_ >= (int)list.size()) selected_ = (int)list.size() - 1;

	MapData* data = (selected_ >= 0) ? &list[selected_] : nullptr;
	if (data) SyncSize(*data);

	const ImGuiStyle& style = ImGui::GetStyle();
	float availH = ImGui::GetContentRegionAvail().y;
	float rowH = availH > 240.0f ? availH : 240.0f;
	float childH = rowH - style.CellPadding.y * 2.0f;

	const ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV;
	if (ImGui::BeginTable("##MapDataEditorCols", 3, flags)) {
		ImGui::TableSetupColumn("MapData", ImGuiTableColumnFlags_WidthStretch, 0.20f);
		ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthStretch, 0.55f);
		ImGui::TableSetupColumn("Detail", ImGuiTableColumnFlags_WidthStretch, 0.25f);
		ImGui::TableNextRow(ImGuiTableRowFlags_None, rowH);

		ImGui::TableSetColumnIndex(0);
		ImGui::BeginChild("##colMapDataList", ImVec2(0, childH), false);
		DrawListColumn(list, log);
		ImGui::EndChild();

		// 一覧で追加・削除された可能性があるので取り直す
		data = (selected_ >= 0 && selected_ < (int)list.size()) ? &list[selected_] : nullptr;
		if (data) SyncSize(*data);

		ImGui::TableSetColumnIndex(1);
		const ImGuiWindowFlags previewFlags = previewHovered_
			? (ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar)
			: ImGuiWindowFlags_None;
		ImGui::BeginChild("##colMapPreview", ImVec2(0, childH), false, previewFlags);
		DrawPreviewColumn(data);
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(2);
		ImGui::BeginChild("##colMapDetail", ImVec2(0, childH), false);
		DrawDetailColumn(data, log);
		ImGui::EndChild();

		ImGui::EndTable();
	}
#else
	(void)list; (void)log;
#endif
}

/// ============================================================
/// 1. MapData 一覧
/// ============================================================
void MapDataEditor::DrawListColumn(std::vector<MapData>& list, std::string& log) {
#ifdef USE_IMGUI
	ImGui::TextDisabled("MAP DATA");
	ImGui::Text("Count: %d", (int)list.size());

	const float bw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f) / 3.0f;
	if (ImGui::Button("New", ImVec2(bw, 0))) {
		std::string name = "NewMap";
		if (newCounter_ > 0) name += "_" + std::to_string(newCounter_);
		newCounter_++;
		list.push_back(CreateMapData(name, kDefaultWidth, kDefaultHeight));
		selected_ = (int)list.size() - 1;
		fitRequested_ = true;
		log += "MapData: created '" + name + "'.\n";
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(selected_ < 0);
	if (ImGui::Button("Duplicate", ImVec2(bw, 0))) {
		MapData copy = list[selected_];
		copy.dataName += "_copy";
		copy.tiles.name = copy.dataName;
		copy.isSaved = false;
		list.push_back(copy);
		selected_ = (int)list.size() - 1;
		fitRequested_ = true;
	}
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));
	if (ImGui::Button("Delete", ImVec2(bw, 0))) {
		log += "MapData: deleted '" + list[selected_].dataName + "'.\n";
		list.erase(list.begin() + selected_);
		selected_ = list.empty() ? -1 : (std::min)(selected_, (int)list.size() - 1);
		fitRequested_ = true;
	}
	ImGui::PopStyleColor(3);
	ImGui::EndDisabled();

	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputTextWithHint("##mapFilter", "Filter (name)", &filter_);

	ImGui::BeginChild("##mapDataList", ImVec2(0, 0), true);
	for (int i = 0; i < (int)list.size(); ++i) {
		const MapData& d = list[i];
		if (!filter_.empty() && !ContainsNoCase(d.dataName, filter_)) continue;

		std::string label = d.dataName + "  (" + std::to_string(d.xSize) + "x" + std::to_string(d.ySize) + ")";
		if (!d.isSaved) label += " *";

		ImGui::PushID(i);
		if (ImGui::Selectable(label.c_str(), selected_ == i)) {
			if (selected_ != i) fitRequested_ = true;
			selected_ = i;
		}
		ImGui::PopID();
	}
	ImGui::EndChild();
#else
	(void)list; (void)log;
#endif
}

/// ============================================================
/// 2. プレビュー
/// ============================================================
void MapDataEditor::DrawPreviewColumn(MapData* data) {
#ifdef USE_IMGUI
	ImGui::TextDisabled("PREVIEW");

	ImGuiIO& io = ImGui::GetIO();
	const float footerH = ImGui::GetFrameHeightWithSpacing() * 2.0f;
	ImVec2 avail = ImGui::GetContentRegionAvail();
	const float w = (std::max)(avail.x, 64.0f);
	const float h = (std::max)(avail.y - footerH, 64.0f);

	const ImVec2 p0 = ImGui::GetCursorScreenPos();
	const ImVec2 p1(p0.x + w, p0.y + h);
	ImDrawList* dl = ImGui::GetWindowDrawList();

	ImGui::InvisibleButton("##mapCanvas", ImVec2(w, h),
		ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
	const bool hovered = ImGui::IsItemHovered();
	const bool active = ImGui::IsItemActive();
	previewHovered_ = hovered || active;

	dl->PushClipRect(p0, p1, true);
	dl->AddRectFilled(p0, p1, IM_COL32(18, 22, 30, 255));

	hoverText_.clear();

	if (!data || data->tiles.Width() == 0 || data->tiles.Height() == 0) {
		const char* msg = data ? "Map size is 0" : "Select or create a MapData";
		ImVec2 ts = ImGui::CalcTextSize(msg);
		dl->AddText(ImVec2(p0.x + (w - ts.x) * 0.5f, p0.y + (h - ts.y) * 0.5f), IM_COL32(150, 160, 180, 255), msg);
		dl->PopClipRect();
		return;
	}

	TileMapData& tiles = data->tiles;
	const int mapW = tiles.Width();
	const int mapH = tiles.Height();

	/// 全体表示
	if (fitRequested_) {
		cellSize_ = std::clamp((std::min)(w / mapW, h / mapH), kMinCell, kMaxCell);
		panX_ = (w - cellSize_ * mapW) * 0.5f;
		panY_ = (h - cellSize_ * mapH) * 0.5f;
		fitRequested_ = false;
	}

	/// ホイール：マウス位置を中心にズーム
	if (hovered && io.MouseWheel != 0.0f) {
		const float oldCell = cellSize_;
		const float factor = io.MouseWheel > 0.0f ? 1.15f : 1.0f / 1.15f;
		cellSize_ = std::clamp(cellSize_ * factor, kMinCell, kMaxCell);
		const float mx = io.MousePos.x - p0.x;
		const float my = io.MousePos.y - p0.y;
		panX_ = mx - (mx - panX_) * (cellSize_ / oldCell);
		panY_ = my - (my - panY_) * (cellSize_ / oldCell);
	}
	/// 右・中ドラッグ：平行移動
	if (active && (ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Middle))) {
		panX_ += io.MouseDelta.x;
		panY_ += io.MouseDelta.y;
	}

	const float originX = p0.x + panX_;
	const float originY = p0.y + panY_;

	/// 画面に見えている範囲だけ描く
	const int x0 = std::clamp((int)std::floor(-panX_ / cellSize_), 0, mapW);
	const int y0 = std::clamp((int)std::floor(-panY_ / cellSize_), 0, mapH);
	const int x1 = std::clamp((int)std::ceil((w - panX_) / cellSize_), 0, mapW);
	const int y1 = std::clamp((int)std::ceil((h - panY_) / cellSize_), 0, mapH);

	for (int y = y0; y < y1; ++y) {
		const auto& row = tiles.Row[y];
		for (int x = x0; x < x1 && x < (int)row.size(); ++x) {
			const ImVec2 a(originX + x * cellSize_, originY + y * cellSize_);
			const ImVec2 b(a.x + cellSize_, a.y + cellSize_);
			dl->AddRectFilled(a, b, TileColor(row[x]));
		}
	}

	/// グリッド（小さすぎる時は描かない）
	if (showGrid_ && cellSize_ >= 6.0f) {
		const ImU32 gridCol = IM_COL32(0, 0, 0, 70);
		for (int x = x0; x <= x1; ++x) {
			const float gx = originX + x * cellSize_;
			dl->AddLine(ImVec2(gx, originY + y0 * cellSize_), ImVec2(gx, originY + y1 * cellSize_), gridCol);
		}
		for (int y = y0; y <= y1; ++y) {
			const float gy = originY + y * cellSize_;
			dl->AddLine(ImVec2(originX + x0 * cellSize_, gy), ImVec2(originX + x1 * cellSize_, gy), gridCol);
		}
	}

	/// マップ外枠
	dl->AddRect(ImVec2(originX, originY),
		ImVec2(originX + mapW * cellSize_, originY + mapH * cellSize_), IM_COL32(200, 200, 220, 160));

	/// ホバー中のマス
	if (hovered || active) {
		const int tx = (int)std::floor((io.MousePos.x - originX) / cellSize_);
		const int ty = (int)std::floor((io.MousePos.y - originY) / cellSize_);
		if (tiles.InBounds(tx, ty)) {
			const ImVec2 a(originX + tx * cellSize_, originY + ty * cellSize_);
			dl->AddRect(a, ImVec2(a.x + cellSize_, a.y + cellSize_), IM_COL32(255, 255, 255, 220), 0.0f, 0, 2.0f);

			char buf[96];
			std::snprintf(buf, sizeof(buf), "(%d, %d)  %s", tx, ty, TileTypeToName(tiles.Row[ty][tx]));
			hoverText_ = buf;

			/// 左ドラッグ：ブラシで塗る
			if (paintEnabled_ && active && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				if (tiles.Row[ty][tx] != brushTile_) {
					tiles.Row[ty][tx] = brushTile_;
					data->isSaved = false;
				}
			}
		}
	}

	dl->PopClipRect();

	/// 下の情報
	ImGui::Text("%dx%d   zoom %.0f px", mapW, mapH, cellSize_);
	ImGui::SameLine();
	ImGui::TextDisabled("%s", hoverText_.c_str());
	if (ImGui::Button("Fit")) fitRequested_ = true;
	ImGui::SameLine();
	ImGui::Checkbox("Grid", &showGrid_);
	ImGui::SameLine();
	ImGui::Checkbox("Paint (LMB)", &paintEnabled_);
	ImGui::SameLine();
	ImGui::TextDisabled("RMB/MMB: pan  Wheel: zoom");
#else
	(void)data;
#endif
}

/// ============================================================
/// 3. 詳細
/// ============================================================
void MapDataEditor::DrawDetailColumn(MapData* data, std::string& log) {
#ifdef USE_IMGUI
	ImGui::TextDisabled("DETAIL");

	if (!data) {
		ImGui::TextDisabled("Select a MapData from the list.");
		return;
	}

	/// ---- 基本情報 ----
	if (ImGui::InputText("Name", &data->dataName)) {
		data->tiles.name = data->dataName;
		data->isSaved = false;
	}

	/// ---- サイズ（Apply を押すまで反映しない） ----
	if (resizeTarget_ != selected_) {
		resizeSize_[0] = data->xSize;
		resizeSize_[1] = data->ySize;
		resizeTarget_ = selected_;
	}
	ImGui::SeparatorText("Size");
	ImGui::Text("Current: %d x %d", data->xSize, data->ySize);
	ImGui::InputInt2("##resize", resizeSize_);
	ImGui::SameLine();
	if (ImGui::Button("Apply##resize")) {
		data->xSize = std::clamp(resizeSize_[0], 1, 4096);
		data->ySize = std::clamp(resizeSize_[1], 1, 4096);
		resizeSize_[0] = data->xSize;
		resizeSize_[1] = data->ySize;
		data->tiles.Resize(data->xSize, data->ySize, AIR);
		data->isSaved = false;
		fitRequested_ = true;
	}
	if (ImGui::Button("Screen (64x36)")) {
		resizeSize_[0] = kDefaultWidth;
		resizeSize_[1] = kDefaultHeight;
	}

	ImGui::SeparatorText("Start Point");
	if (ImGui::InputInt("Start X", &data->startX)) data->isSaved = false;
	if (ImGui::InputInt("Start Y", &data->startY)) data->isSaved = false;

	/// ---- パレット ----
	ImGui::SeparatorText("Tile Palette");
	for (int t = AIR; t < MAX_TYPE; ++t) {
		ImGui::PushID(t);
		const ImU32 c = TileColor(t);
		const ImVec4 col = ImGui::ColorConvertU32ToFloat4(c);
		ImGui::ColorButton("##swatch", col, ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
		ImGui::SameLine();
		if (ImGui::RadioButton(TileTypeToName(t), brushTile_ == t)) brushTile_ = t;
		ImGui::PopID();
	}

	char fillLabel[64];
	std::snprintf(fillLabel, sizeof(fillLabel), "Fill all with %s", TileTypeToName(brushTile_));
	if (ImGui::Button(fillLabel, ImVec2(-FLT_MIN, 0))) {
		data->tiles.Fill(brushTile_);
		data->isSaved = false;
	}
	if (ImGui::Button("Clear (Air)", ImVec2(-FLT_MIN, 0))) {
		data->tiles.Fill(AIR);
		data->isSaved = false;
	}

	/// ---- CSV ----
	ImGui::SeparatorText("CSV");
	if (csvPath_.empty()) {
		csvPath_ = FileManager::EnsureSlash(FileManager::GetDesktopPath()) + "map.csv";
	}
	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputText("##csvPath", &csvPath_);

	const float half = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
	if (ImGui::Button("Export CSV", ImVec2(half, 0))) {
		if (data->tiles.ExportCsv(csvPath_)) {
			data->isSaved = true;
			log += "MapData: exported '" + data->dataName + "' -> " + csvPath_ + "\n";
		} else {
			log += "MapData Error: cannot write " + csvPath_ + "\n";
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Import CSV", ImVec2(half, 0))) {
		if (data->tiles.ImportCsv(csvPath_)) {
			data->xSize = data->tiles.Width();
			data->ySize = data->tiles.Height();
			resizeTarget_ = -1;
			data->isSaved = true;
			fitRequested_ = true;
			log += "MapData: imported " + csvPath_ + "\n";
		} else {
			log += "MapData Error: cannot read " + csvPath_ + "\n";
		}
	}
#else
	(void)data; (void)log;
#endif
}
