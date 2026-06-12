#include "MapMakingEditor.h"
#include "DebugDraw.h"
#include "ImguiManager.h"

MapMakingEditor::MapMakingEditor(kEngine* system) {
	system_ = system;
}

MapMakingEditor::~MapMakingEditor() {}

void MapMakingEditor::Update() {
	// Implementation for update logic
}

void MapMakingEditor::Draw() {
	ImGuiPart();
}

void MapMakingEditor::ImGuiPart() {

	ImGuiIO& io = ImGui::GetIO();
	menuBarHeight_ = ImGui::GetFrameHeight();

	/// メニューバー
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File")) {
			ImGui::MenuItem("New");
			ImGui::MenuItem("Open");
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	{

		/// デバッグウィンドウ
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - inspectorWidth_, menuBarHeight_));
		ImGui::SetNextWindowSize(ImVec2(inspectorWidth_, io.DisplaySize.y - menuBarHeight_));
		ImGui::Begin("Inspector",
			nullptr,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse);   // ← 移除 NoResize
		// 更新寬度（使用 ImGui 的 API 取得目前視窗大小）
		inspectorWidth_ = ImGui::GetWindowSize().x;
		ImGui::Text("This is a debug window.");
		ImGui::End();
	}
}