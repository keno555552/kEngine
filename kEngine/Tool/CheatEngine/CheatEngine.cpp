#include "CheatEngine.h"
#include "kEngine.h"
#include <fstream>
#include "externals/nlohmann/json.hpp"

CheatEngine::~CheatEngine() { Save(); }

void CheatEngine::Initialize(kEngine* system) {
	system_ = system;
	objectEditor_.Initialize(system);
	Load();
}

/// =================== 設定保存関連 =================== ///

void CheatEngine::Load() {
	std::ifstream file(kSettingsPath);
	if (!file.is_open()) return;                   // 沒檔案就用預設值，第一次存檔時才生成

	nlohmann::json j = nlohmann::json::parse(file, nullptr, false);   // 不拋例外
	if (j.is_discarded() || !j.is_object()) return;

	isOpen_ = j.value("menuOpen", false);
	freezeScene_ = j.value("freezeScene", false);
	isObjectEditorOn_ = j.value("isObjectEditorOn", false);
	saved_ = { isOpen_, freezeScene_, isObjectEditorOn_ };
}

void CheatEngine::Save() {
	Settings now{ isOpen_, freezeScene_, isObjectEditorOn_ };
	if (now == saved_) return;                     // 変化がなければ保存しない

	nlohmann::json j;
	j["menuOpen"] = isOpen_;
	j["freezeScene"] = freezeScene_;
	j["isObjectEditorOn"] = isObjectEditorOn_;

	std::ofstream file(kSettingsPath);
	if (!file.is_open()) return;
	file << j.dump(4);
	saved_ = now;
}

/// ====================== 描画 ====================== ///
void CheatEngine::Draw() {
	// F3 切換（文字輸入中不處理）
	if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_F3, false)) {
		isOpen_ = !isOpen_;
	}

	if (isOpen_) {
		ImGui::SetNextWindowSize(ImVec2(420, 520), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Cheat Menu (F3)", &isOpen_)) {

			if (ImGui::CollapsingHeader("System", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Text("FPS: %.1f", system_->GetFPS());
				ImGui::Text("DeltaTime: %.4f", system_->GetDeltaTime());
			}

			if (ImGui::CollapsingHeader("Playing Speed", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Checkbox("Freeze Scene Update", &freezeScene_);
			}

			if (ImGui::CollapsingHeader("ObjectsEditor", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Checkbox("On/Off", &isObjectEditorOn_);
			}

		}
		ImGui::End();

		if (isObjectEditorOn_) {
			ImGui::SetNextWindowSize(ImVec2(1280, 620), ImGuiCond_FirstUseEver);
			ImGui::Begin("Object Editor", &isObjectEditorOn_);
			objectEditor_.Draw();
			ImGui::End();
		}
	}
	if (!(isOpen_ && isObjectEditorOn_)) {
		objectEditor_.ReleaseSolo();
	}
	Save();
}