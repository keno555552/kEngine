#include "MapMakingEditor.h"
#include "DebugDraw.h"
#include "ImguiManager.h"

MapMakingEditor::MapMakingEditor(kEngine* system) {
    system_ = system;
}

MapMakingEditor::~MapMakingEditor() {

}


void MapMakingEditor::Update() {
    // Implementation for update logic
}

void MapMakingEditor::Draw() {
    ImGuiPart();
}

void MapMakingEditor::ImGuiPart() {

    ImGui::Begin("Map Editor", nullptr, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()){
        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::MenuItem("Open Inspector"));
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::End();

}   