#pragma once
#include "BaseScene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "Lighting\Light.h"
#include "Camera\DebugCamera.h"
#include "GAME\Object\DetailButton\DetailButton.h"
#include "GameObject\button\button.h"
#include "Tool\DefaultMenu\DefaultMenu.h"

class MapMakingEditor : public BaseScene
{
public:
	MapMakingEditor(kEngine* system);
	~MapMakingEditor();

	void Update() override;
	void Draw() override;

private:

	float inspectorWidth_ = 300.0f;
	float menuBarHeight_ = 0.0f;

	bool showLoadWindow_ = false;
	bool showSaveWindow_ = false;

	std::string codeingText_;

	std::string loadFilePath_;
	std::string saveFilePath_;

private:
	void ImGuiPart();
	void ImGuiLoadWindow();
	void ImGuiSaveWindow();
	bool InputTextString(const char* label, std::string& str, ImGuiInputTextFlags flags = 0);
	bool InputBigTextString(const char* label, std::string& str, ImGuiInputTextFlags flags = 0);

};

