#pragma once
#include <string>
#include <vector>
#include "CheatEngine/ObjectEditor/ObjectEditor.h"
#include "DebugTool/Logger.h"

class kEngine;

class CheatEngine {
public:
	CheatEngine() = default;
	~CheatEngine();

	void Initialize(kEngine* system);
	void Draw();

	bool IsSceneFrozen() const { return isOpen_ && freezeScene_; }

	/// ======= ゲームローグ ======= ///
	static void GameLog(const std::string& message);
	template<typename... Args>
	static void GameLog(const char* fmt, Args... args) {
		int size = std::snprintf(nullptr, 0, fmt, args...) + 1;
		if (size <= 0) return;
		std::vector<char> buf(size);
		std::snprintf(buf.data(), size, fmt, args...);
		GameLog(std::string(buf.data()));
	}
	static void ClearGameLog() { gameLogEntries_.clear(); }

private:
	struct Settings {
		bool isOpen = false;
		bool freezeScene = false;
		bool isObjectEditorOn = false;
		bool isGameLogOn = false;
		bool operator==(const Settings&) const = default;
	};

	void Load();
	void Save();

	/// == システム == ///
	kEngine* system_ = nullptr;
	ObjectEditor objectEditor_{};

	/// ======== 設定 ======== ///
	bool isOpen_ = false;
	bool freezeScene_ = false;
	bool isObjectEditorOn_ = false;
	Settings saved_{};

	/// ===== Log ===== ///
	inline static std::vector<Entry> gameLogEntries_;
	static constexpr size_t kMaxGameLog = 1000;

	bool isLogOn_ = false;
	bool logAutoScroll_ = true;
	char logFilter_[128] = {};

	static constexpr const char* kSettingsPath = "./cheat_settings.json";

private:
	void DrawLogWindow();
	void DrawLogList(const std::vector<Entry>& entries, const char* childId);
};

