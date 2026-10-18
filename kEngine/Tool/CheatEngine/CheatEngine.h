#pragma once
#include <string>
#include "CheatEngine/ObjectEditor/ObjectEditor.h"

class kEngine;

class CheatEngine {
public:
	CheatEngine() = default;
	~CheatEngine();

	void Initialize(kEngine* system);
	void Draw();

	bool IsSceneFrozen() const { return isOpen_ && freezeScene_; }

private:
	struct Settings {
		bool isOpen = false;
		bool freezeScene = false;
		bool isObjectEditorOn = false;
		bool operator==(const Settings&) const = default;
	};

	void Load();
	void Save();

	/// ==	=====
	kEngine* system_ = nullptr;
	ObjectEditor objectEditor_{};

	/// ======= 
	bool isOpen_ = false;
	bool freezeScene_ = false;
	bool isObjectEditorOn_ = false;
	Settings saved_{};

	static constexpr const char* kSettingsPath = "./cheat_settings.json";
};

