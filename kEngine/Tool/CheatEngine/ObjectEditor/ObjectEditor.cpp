#include "ObjectEditor.h"
#include "CheatEngine/ObjectEditor/ObjectRegistrant.h"
#include "Object/Object.h"
#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cstdio>
#include <string>
#include <typeinfo>
#include "kEngine.h"

#include "ImGuiManager.h"


namespace {
	std::string MakeLabel(int index, Object* obj) {
		return "[" + std::to_string(index) + "] " + typeid(*obj).name()
			+ "  (model " + std::to_string(obj->modelHandle_) + ")";
	}

	bool ContainsNoCase(const std::string& text, const std::string& key) {
		auto it = std::search(text.begin(), text.end(), key.begin(), key.end(),
			[](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
		return it != text.end();
	}

	DrawDataCollector::PreviewView MakeFreeView(const Vector3& pos, const Vector3& rot, float aspect) {
		DrawDataCollector::PreviewView v{};
		v.view = Inverse(MakeAffineMatrix(Vector3{ 1.0f, 1.0f, 1.0f }, rot, pos));
		v.proj = MakePerspectiveFovMatrix(0.7f, aspect, 0.05f, 500.0f);
		v.position = pos;
		return v;
	}
}

void ObjectEditor::FocusSelected() {
	const Vector3 target = selected_ ? selected_->mainPosition.transform.translate : Vector3{ 0.0f, 0.0f, 0.0f };
	const float dist = 4.0f, height = 1.0f;
	camPos_ = Vector3{ target.x, target.y + height, target.z - dist };
	camRot_ = Vector3{ atan2f(height, dist), 0.0f, 0.0f };
}

void ObjectEditor::UpdateFreeCamera() {
	ImGuiIO& io = ImGui::GetIO();
	const bool hovered = ImGui::IsItemHovered();
	const bool active = ImGui::IsItemActive();
	previewHovered_ = hovered || active;
	if (!previewHovered_) return;           // ホイールで親がスクロールしないように

	Vector3 move{ 0.0f, 0.0f, 0.0f };

	/// キーボード（DebugCameraと同じ割り当て）
	if (!io.WantTextInput) {
		const float step = 0.6f * io.DeltaTime;
		if (ImGui::IsKeyDown(ImGuiKey_Q)) move.z += step;
		if (ImGui::IsKeyDown(ImGuiKey_E)) move.z -= step;
		if (ImGui::IsKeyDown(ImGuiKey_A)) move.x -= step;
		if (ImGui::IsKeyDown(ImGuiKey_D)) move.x += step;
		if (ImGui::IsKeyDown(ImGuiKey_W)) move.y += step;
		if (ImGui::IsKeyDown(ImGuiKey_S)) move.y -= step;
		if (ImGui::IsKeyPressed(ImGuiKey_R, false)) FocusSelected();
	}

	/// 右ドラッグ: 視点回転
	if (active && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
		camRot_.x += io.MouseDelta.y * 0.005f;
		camRot_.y += io.MouseDelta.x * 0.005f;
		camRot_.x = std::clamp(camRot_.x, -1.55f, 1.55f);
	}
	/// 中ドラッグ: 平行移動
	if (active && ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
		move.x += io.MouseDelta.x * -0.005f;
		move.y += io.MouseDelta.y * 0.005f;
	}
	/// ホイール: 前後
	move.z += io.MouseWheel * 0.5f;

	/// ローカル移動をカメラの向きでワールドに変換（DebugCameraと同じ方法）
	Matrix4x4 handle = MakeTranslateMatrix4x4(move) * MakeRotateMatrix4x4(camRot_);
	camPos_ = camPos_ + MakeTranslateVector3(handle);
}

void ObjectEditor::SubmitPreviewRequest() {
	if (!system_) return;
	DrawEngine* de = system_->GetDrawEngine();
	previewTexture_ = de->GetPreviewTextureID();

	DrawEngine::PreviewRequest req;
	req.active = true;
	req.object = selected_;
	if (selected_) {
		req.partIndex = (isolatePart_ && selectedPart_ >= 0 && selectedPart_ < (int)selected_->objectParts_.size())
			? selectedPart_ : -1;
		req.view = MakeFreeView(camPos_, camRot_, 512.0f / 384.0f);
	}
	de->SetPreviewRequest(req);
}

void ObjectEditor::ReleaseSolo() {
	if (!system_) return;
	if (DrawDataCollector* ddc = system_->GetDrawDataCollector()) {
		ddc->ClearSoloTarget();
	}
}

void ObjectEditor::Draw() {
	const auto& objs = ObjectRegistrant::GetInstance().GetObjects();

	// 選取中のオブジェクトが破棄されていたら解除
	if (selected_ && std::find(objs.begin(), objs.end(), selected_) == objs.end()) {
		ClearSelection();
		selectedPart_ = -1;
	}

	/// 選択が変わったら、そのオブジェクトの正面にカメラを移す
	if (selected_ != focusedObject_) {
		focusedObject_ = selected_;
		if (selected_) FocusSelected();
	}


	ApplySolo();

	const ImGuiStyle& style = ImGui::GetStyle();
	float availH = ImGui::GetContentRegionAvail().y;
	float rowH = availH > 240.0f ? availH : 240.0f;
	float childH = rowH - style.CellPadding.y * 2.0f;

	const ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV;
	if (ImGui::BeginTable("##ObjectEditorCols", 3, flags)) {
		ImGui::TableSetupColumn("Objects", ImGuiTableColumnFlags_WidthStretch, 0.26f);
		ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthStretch, 0.38f);
		ImGui::TableSetupColumn("Detail", ImGuiTableColumnFlags_WidthStretch, 0.36f);

		ImGui::TableNextRow(ImGuiTableRowFlags_None, rowH);

		ImGui::TableSetColumnIndex(0);
		ImGui::BeginChild("##colObjects", ImVec2(0, childH), false);
		DrawObjectColumn();
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(1);
		const ImGuiWindowFlags previewFlags = previewHovered_
			? (ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar)
			: ImGuiWindowFlags_None;
		ImGui::BeginChild("##colPreview", ImVec2(0, childH), false, previewFlags);
		DrawPreviewColumn();
		ImGui::EndChild();

		ImGui::TableSetColumnIndex(2);
		ImGui::BeginChild("##colDetail", ImVec2(0, childH), false);
		DrawDetailColumn();
		ImGui::EndChild();

		ImGui::EndTable();
	}
	SubmitPreviewRequest();
}

void ObjectEditor::ApplySolo() {
	if (!system_) return;
	DrawDataCollector* ddc = system_->GetDrawDataCollector();
	if (!ddc) return;

	//if (soloEnabled_ && selected_) {
	//	int part = -1;
	//	if (isolatePart_ && selectedPart_ >= 0 && selectedPart_ < (int)selected_->objectParts_.size()) {
	//		part = selectedPart_;
	//	}
	//	ddc->SetSoloTarget(selected_, part);
	//} else {
	//	ddc->ClearSoloTarget();
	//}

	if (soloEnabled_ && selected_) {
		ddc->SetSoloTarget(selected_, -1);
	} else {
		ddc->ClearSoloTarget();
	}
}

/// ============================================================
/// 1. Object総覧リスト
/// ============================================================
void ObjectEditor::DrawObjectColumn() {
	const auto& objs = ObjectRegistrant::GetInstance().GetObjects();

	ImGui::TextDisabled("OBJECTS");
	ImGui::Text("Objects: %d", (int)objs.size());

	ImGui::SetNextItemWidth(-FLT_MIN);
	ImGui::InputTextWithHint("##filter", "Filter (class name)", &filter_);

	float listH = ImGui::GetContentRegionAvail().y - 150.0f;
	if (listH < 80.0f) listH = 80.0f;

	ImGui::BeginChild("##objectList", ImVec2(0, listH), true);
	for (int i = 0; i < (int)objs.size(); ++i) {
		Object* obj = objs[i];
		std::string label = MakeLabel(i, obj);
		if (!filter_.empty() && !ContainsNoCase(label, filter_)) continue;

		ImGui::PushID(obj);
		if (ImGui::Selectable(label.c_str(), selected_ == obj)) {
			selected_ = obj;
			selectedPart_ = -1;
		}
		ImGui::PopID();
	}
	ImGui::EndChild();

	if (selected_) {
		ImGui::TextDisabled("Selected: %s", typeid(*selected_).name());
	} else {
		ImGui::TextDisabled("Selected: (none)");
	}

	if (ImGui::Button("Deselect (show all)")) {
		ClearSelection();
		selectedPart_ = -1;
	}
	ImGui::Checkbox("Solo in game view", &soloEnabled_);

	// まだ機能がないもの
	ImGui::BeginDisabled();
	bool dummy = false;
	ImGui::Button("Select by click");
	ImGui::SameLine();
	ImGui::Button("Hide");
	ImGui::Checkbox("Show/Hide all  (next)", &dummy);
	ImGui::EndDisabled();
}

/// ============================================================
/// 2. 閲覧用のプレビュー
/// ============================================================
void ObjectEditor::DrawPreviewColumn() {
	ImGui::TextDisabled("PREVIEW");

	ImVec2 avail = ImGui::GetContentRegionAvail();
	float w = avail.x;
	float h = w * 0.75f;                          // 512x384 の比率
	float maxH = avail.y - 190.0f;                // 下のコントロール分を残す
	if (h > maxH) h = maxH;
	if (h < 120.0f) h = 120.0f;

	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* dl = ImGui::GetWindowDrawList();

	if (previewTexture_) {
		ImGui::Image((ImTextureID)previewTexture_, ImVec2(w, h));
		ImGui::SetCursorScreenPos(p);
		ImGui::InvisibleButton("##previewView", ImVec2(w, h),
			ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
		UpdateFreeCamera();
	} else {
		// render target がまだつながっていない間のプレースホルダー
		ImVec2 q(p.x + w, p.y + h);
		dl->AddRectFilledMultiColor(p, q,
			IM_COL32(35, 43, 58, 255), IM_COL32(35, 43, 58, 255),
			IM_COL32(18, 22, 30, 255), IM_COL32(18, 22, 30, 255));
		dl->AddRect(p, q, IM_COL32(61, 61, 74, 255));

		const char* msg = "Preview (render target not connected)";
		ImVec2 ts = ImGui::CalcTextSize(msg);
		dl->AddText(ImVec2(p.x + (w - ts.x) * 0.5f, p.y + (h - ts.y) * 0.5f),
			IM_COL32(150, 160, 180, 255), msg);
		ImGui::Dummy(ImVec2(w, h));
	}

	// 左上に選択中のオブジェクト名
	if (selected_) {
		char buf[128];
		std::snprintf(buf, sizeof(buf), "%s   parts: %d",
			typeid(*selected_).name(), (int)selected_->objectParts_.size());
		dl->AddText(ImVec2(p.x + 8.0f, p.y + 6.0f), IM_COL32(170, 180, 198, 255), buf);
	}

	ImGui::Spacing();

	// ハイライト対象のパーツ名
	ImGui::Checkbox("Highlight selected part", &isolatePart_);
	if (selected_ && selectedPart_ >= 0 && selectedPart_ < (int)selected_->objectParts_.size()) {
		ImGui::SameLine();
		ImGui::TextDisabled("-> %s", selected_->objectParts_[selectedPart_].name.c_str());
	}
	ImGui::Checkbox("Wireframe", &showWireframe_);
	ImGui::Checkbox("Show Axis", &showAxis_);

	ImGui::DragFloat3("Position", &camPos_.x, 0.05f);
	ImGui::DragFloat2("Rotate (X,Y)", &camRot_.x, 0.01f);
	if (ImGui::Button("Focus selected (R)")) FocusSelected();
	ImGui::TextDisabled("RMB drag: look   MMB drag: pan   Wheel: zoom");
	ImGui::TextDisabled("Hover + WASD/QE: move   R: focus");
}

/// ============================================================
/// 3. Object内部partの詳細情報
/// ============================================================
void ObjectEditor::DrawDetailColumn() {
	ImGui::TextDisabled("DETAIL");

	if (!selected_) {
		ImGui::TextDisabled("Select an object from the list.");
		return;
	}
	Object* obj = selected_;

	ImGui::Text("%s", typeid(*obj).name());
	ImGui::Text("modelHandle: %d   parts: %d", obj->modelHandle_, (int)obj->objectParts_.size());
	ImGui::Text("follow: %s", obj->followObject_ ? "yes" : "no");

	ImGui::Checkbox("isBillboard", &obj->isBillboard_);
	ImGui::Checkbox("isControlledByAnimation", &obj->isControlledByAnimation_);
	ImGui::Checkbox("isDelete", &obj->isDelete_);

	if (ImGui::CollapsingHeader("mainPosition", ImGuiTreeNodeFlags_DefaultOpen)) {
		DrawTransformBody(obj->mainPosition.transform);
	}

	char header[64];
	std::snprintf(header, sizeof(header), "Parts (%d)", (int)obj->objectParts_.size());
	if (ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_DefaultOpen)) {
		for (int i = 0; i < (int)obj->objectParts_.size(); ++i) {
			auto& part = obj->objectParts_[i];

			ImGui::PushID(i);
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (selectedPart_ == i) flags |= ImGuiTreeNodeFlags_Selected;

			bool open = ImGui::TreeNodeEx("##part", flags, "part[%d] %s", i, part.name.c_str());
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
				selectedPart_ = (selectedPart_ == i) ? -1 : i;       // 中央のプレビューでハイライトする対象
			}
			if (open) {
				DrawTransformBody(part.transform);
				DrawUVTransformBody(*part.materialConfig);
				ImGui::TextDisabled("Material  (next step)");
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
}

void ObjectEditor::DrawTransformBody(Transform& t) {
	ImGui::DragFloat3("Translate", &t.translate.x, 0.05f);
	ImGui::DragFloat3("Rotate", &t.rotate.x, 0.01f);
	ImGui::DragFloat3("Scale", &t.scale.x, 0.01f);
}

void ObjectEditor::DrawUVTransformBody(MaterialConfig& m) {
	ImGui::SeparatorText("UV Transform");
	ImGui::DragFloat3("UV Translate", &m.uvTranslate.x, 0.005f);
	ImGui::DragFloat3("UV Scale", &m.uvScale.x, 0.01f);
	ImGui::DragFloat("UV Rotate", &m.uvRotate.z, 0.01f);
	if (ImGui::Button("Reset UV")) {
		m.uvTranslate = Vector3{ 0.0f, 0.0f, 0.0f };
		m.uvScale = Vector3{ 1.0f, 1.0f, 1.0f };
		m.uvRotate = Vector3{ 0.0f, 0.0f, 0.0f };
	}
}