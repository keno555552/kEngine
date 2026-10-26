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

/// ================= ゲームローグ関連 ================== ///
void CheatEngine::GameLog(const std::string& message) {
	/// Logger にも送る → 出力ウィンドウ・ファイル・クラッシュログに載る
	Logger::Log("[Game] " + message + "\n");

	/// 今 Logger が作った 1 件をそのまま使う（時刻が完全に同じになる）
	Entry entry = Logger::GetEntries().back();
	entry.saveLog = message;                       // 画面では [Game] を付けない
	gameLogEntries_.push_back(std::move(entry));

	if (gameLogEntries_.size() > kMaxGameLog) {
		gameLogEntries_.erase(gameLogEntries_.begin());
	}
}

/// =================== 設定保存関連 =================== ///

void CheatEngine::Load() {
	std::ifstream file(kSettingsPath);
	if (!file.is_open()) return;                   // 沒檔案就用預設值，第一次存檔時才生成

	nlohmann::json j = nlohmann::json::parse(file, nullptr, false);   // 不拋例外
	if (j.is_discarded() || !j.is_object()) return;

	/// 各設定を JSON から読み込む
	isOpen_ = j.value("menuOpen", false);
	freezeScene_ = j.value("freezeScene", false);
	isObjectEditorOn_ = j.value("isObjectEditorOn", false);
	isLogOn_ = j.value("isLogOn", false);

	/// 保存済み設定を更新
	saved_ = { isOpen_, freezeScene_, isObjectEditorOn_, isLogOn_ };
}

void CheatEngine::Save() {
	Settings now{ isOpen_, freezeScene_, isObjectEditorOn_, isLogOn_ };
	if (now == saved_) return;                     // 変化がなければ保存しない

	/// 各設定を JSON に書き込む
	nlohmann::json j;
	j["menuOpen"] = isOpen_;
	j["freezeScene"] = freezeScene_;
	j["isObjectEditorOn"] = isObjectEditorOn_;
	j["isLogOn"] = isLogOn_;

	/// ファイルに保存
	std::ofstream file(kSettingsPath);
	if (!file.is_open()) return;
	file << j.dump(4);
	saved_ = now;

}

/// ====================== 描画 ====================== ///

void CheatEngine::DrawLogWindow() {
	ImGui::SetNextWindowSize(ImVec2(720, 360), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Log", &isLogOn_)) { ImGui::End(); return; }

	/// ===== 共通ツールバー ===== ///
	ImGui::Checkbox("Auto Scroll", &logAutoScroll_);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(-1);
	ImGui::InputTextWithHint("##logFilter", "filter", logFilter_, sizeof(logFilter_));

	if (ImGui::BeginTabBar("##logTabs")) {
		if (ImGui::BeginTabItem("Game")) {
			if (ImGui::Button("Clear")) ClearGameLog();
			DrawLogList(gameLogEntries_, "##gameLog");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("System")) {
			if (ImGui::Button("Clear")) Logger::ClearEntries();
			DrawLogList(Logger::GetEntries(), "##systemLog");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();
}

void CheatEngine::DrawLogList(const std::vector<Entry>& entries, const char* childId) {
	/// フィルターに合う行だけ選ぶ
	std::vector<int> shown;
	shown.reserve(entries.size());
	for (int i = 0; i < (int)entries.size(); ++i) {
		if (logFilter_[0] != '\0' && entries[i].saveLog.find(logFilter_) == std::string::npos) continue;
		shown.push_back(i);
	}

	/// 見えている行だけ描く
	ImGui::BeginChild(childId, ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	ImGuiListClipper clipper;
	clipper.Begin((int)shown.size());
	while (clipper.Step()) {
		for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
			const Entry& e = entries[shown[row]];
			const bool isError = e.saveLog.find("Error") != std::string::npos;
			const ImVec4 color = isError ? ImVec4(1.0f, 0.4f, 0.4f, 1.0f) : ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
			ImGui::TextColored(color, "[%s] %s", e.time.c_str(), e.saveLog.c_str());
		}
	}
	if (logAutoScroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
	ImGui::EndChild();
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

			if (ImGui::CollapsingHeader("Log", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Checkbox("Log Window", &isLogOn_);
				ImGui::TextDisabled("%s", Logger::GetLogFilePath().c_str());
			}
		}
		ImGui::End();

		if (isObjectEditorOn_) {
			ImGui::SetNextWindowSize(ImVec2(1280, 620), ImGuiCond_FirstUseEver);
			ImGui::Begin("Object Editor", &isObjectEditorOn_);
			objectEditor_.Draw();
			ImGui::End();
		}

		if (isLogOn_) DrawLogWindow();
	}
	if (!(isOpen_ && isObjectEditorOn_)) {
		objectEditor_.ReleaseSolo();
	}
	Save();
}
