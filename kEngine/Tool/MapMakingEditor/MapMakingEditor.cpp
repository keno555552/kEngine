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

#ifdef USE_IMGUI
void MapMakingEditor::ImGuiPart() {

	ImGuiIO& io = ImGui::GetIO();
	menuBarHeight_ = ImGui::GetFrameHeight();

	/// ============== メニューバー =============== ///
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {};
			if (ImGui::MenuItem("Load"))showLoadWindow_ = true;
			if (ImGui::MenuItem("Save"))showSaveWindow_ = true;
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
	ImGuiNewWindow();
}

void MapMakingEditor::ImGuiLeftMenuBar() {
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 w1Pos{};
	ImVec2 w1Size{};

	float selectorH = menuBarHeight_;
	float lineH = ImGui::GetTextLineHeightWithSpacing();
	float titleH = ImGui::GetFrameHeight();



	float templateLines = 1;
	float templateH = templateLines * lineH + 20.0f + titleH;

	float selectorPosY = menuBarHeight_;
	float templatePosY = io.DisplaySize.y - templateH;

	// =========================
	// 1. Top Selector
	// =========================
	ImGui::SetNextWindowPos(ImVec2(0, selectorPosY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, selectorH));

	ImGui::Begin("##LeftSelector",
		nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse);

	if (ImGui::Selectable("Terrain", modeEditing_ == 0, 0, ImVec2(80, selectorH - 4)))
		modeEditing_ = 0;
	ImGui::SameLine();
	if (ImGui::Selectable("Room", modeEditing_ == 1, 0, ImVec2(80, selectorH - 4)))
		modeEditing_ = 1;
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Testing");

	w1Pos = ImGui::GetWindowPos();
	w1Size = ImGui::GetWindowSize();

	ImGui::End();

	// ======== pos計算 =========
	float MapH =
		lineH * 7 +
		ImGui::GetFrameHeight() +
		10.0f +
		titleH;

	float colimenH = (io.DisplaySize.y - w1Size.y - selectorH - templateH - titleH * 2) / 3;
	float window2H = 1;
	float window3H = 1;
	float window4H = titleH;

	enum {
		Map = 1 << 0,
		Generator = 1 << 1,
		Variables = 1 << 2,
	};

	int nowState = {
		(isMapWindowOpen_ ? Map : 0) |
		(isGenratorWindowOpen_ ? Generator : 0) |
		(isVariablesWindowOpen_ ? Variables : 0)
	};

	//switch (nowState) {
	//case Map:
	//
	//	window2H = colimenH * 3;
	//	break;
	//
	//case Map | Generator:
	//
	//	window2H = colimenH;
	//	window3H = colimenH * 2 + titleH;
	//	break;
	//
	//case Generator:
	//
	//	window3H = colimenH * 3;
	//
	//	break;
	//
	//case Generator | Variables:
	//
	//	window3H = colimenH;
	//	window4H = colimenH * 2 + titleH;
	//	break;
	//
	//case Variables:
	//
	//	window4H = colimenH * 3;
	//	break;
	//
	//case Map | Variables:
	//
	//	window2H = colimenH;
	//	window4H = colimenH * 2 + titleH;
	//	break;
	//
	//case Map | Generator | Variables:
	//
	//	window2H = colimenH;
	//	window3H = colimenH + titleH;
	//	window4H = colimenH + titleH;
	//	break;
	//}

	int maxMidWindowCount = 3;
	int windowCount = 0;
	if(nowState & Map)			windowCount++;
	if(nowState & Generator)	windowCount++;
	if(nowState & Variables)	windowCount++;
	int windowLeft = windowCount;
	 
	if (nowState & Map) {
		window2H = colimenH;
		windowLeft--;
		if (windowLeft == 0) window2H += colimenH * (maxMidWindowCount - windowCount);
	}
	if (nowState & Generator)	{
		window3H = colimenH;
		if (nowState & Map) window3H += titleH;
		windowLeft--;
		if (windowLeft == 0) window3H += colimenH  * (maxMidWindowCount - windowCount);
	}
	if (nowState & Variables) {
		window4H = colimenH ;
		if (nowState & Map)				window4H += titleH;
		else if (nowState & Generator)	window4H += titleH;
		windowLeft--;
		if (windowLeft == 0) window4H += colimenH * (maxMidWindowCount - windowCount);
	}

	// =========================
	// 2. Maps（獨立視窗）
	// =========================
	float window2Y = w1Pos.y + w1Size.y;
	ImVec2 w2Pos;
	ImVec2 w2Size;

	ImGui::SetNextWindowPos(ImVec2(0, window2Y));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, window2H));

	//ImGui::SetNextWindowCollapsed(false, ImGuiCond_FirstUseEver);
	isMapWindowOpen_ = ImGui::Begin("Maps",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float fullW = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float addW = fullW * 0.7f;
	float delW = fullW * 0.3f;

	if (ImGui::Button("Add Map", ImVec2(addW, 20))) {
		showNewWindow_ = true;
		std::string newNumber = std::to_string(newCounter_);
		if (newCounter_ == 0)newNumber = "";
		newMapName_ = "NewMap" + newNumber;
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(delW, 20))) {
		if (selectedMap_ >= 0 && selectedMap_ < mapData_.size()) {
			mapData_.erase(mapData_.begin() + selectedMap_);
			selectedMap_ = -1;
		}
	}

	ImGui::PopStyleColor(3);

	ImGui::BeginChild("MapList", ImVec2(0, window2H - titleH - ImGui::GetFrameHeight() - 20), true);
	for (int i = 0; i < mapData_.size(); i++) {
		auto& m = mapData_[i];
		std::string label = m->tileMapData.name + (m->isSaved ? "" : " *");

		if (ImGui::Selectable(label.c_str(), selectedMap_ == i)) {
			selectedMap_ = i;
		}
	}
	ImGui::EndChild();

	w2Pos = ImGui::GetWindowPos();
	w2Size = ImGui::GetWindowSize();

	ImGui::End();


	// =========================
	// 3. Generator List（獨立視窗）
	// =========================

	float window3Y = w2Pos.y + w2Size.y;
	ImVec2 w3Pos;
	ImVec2 w3Size;

	ImGui::SetNextWindowPos(ImVec2(0, window3Y));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, window3H));

	isGenratorWindowOpen_ = ImGui::Begin("Generator List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float full2W = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float add2W = full2W * 0.7f;
	float del2W = full2W * 0.3f;

	if (ImGui::Button("Add Generator", ImVec2(add2W, 20))) {
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(del2W, 20))) {
	}

	ImGui::PopStyleColor(3);

	ImGui::BeginChild("GeneratorList", ImVec2(0, window3H - titleH - ImGui::GetFrameHeight() - 20), true);
	for (int i = 0; i < 10; i++)
		ImGui::Selectable(("Generator " + std::to_string(i)).c_str());
	ImGui::EndChild();

	w3Pos = ImGui::GetWindowPos();
	w3Size = ImGui::GetWindowSize();

	ImGui::End();


	// =========================
	// 4. Variable List（獨立視窗）
	// =========================

	float window4Y = w3Pos.y + w3Size.y;
	ImVec2 w4Pos;
	ImVec2 w4Size;

	ImGui::SetNextWindowPos(ImVec2(0, window4Y));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, window4H));

	isVariablesWindowOpen_ = ImGui::Begin("Variables List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float full3W = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float add3W = full3W * 0.7f;
	float del3W = full3W * 0.3f;

	if (ImGui::Button("Add Variables", ImVec2(add3W, 20))) {
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(del3W, 20))) {
	}

	ImGui::PopStyleColor(3);

	ImGui::BeginChild("VariableList", ImVec2(0, window4H - titleH - ImGui::GetFrameHeight() - 20), true);
	for (int i = 0; i < 10; i++)
		ImGui::Selectable(("Variable " + std::to_string(i)).c_str());
	ImGui::EndChild();

	w4Pos = ImGui::GetWindowPos();
	w4Size = ImGui::GetWindowSize();

	ImGui::End();


	// =========================
	// 4. Template Info（獨立視窗）
	// =========================

	if (nowState == 0) {
		templatePosY = w4Pos.y + w4Size.y;
		templateH = io.DisplaySize.y - templatePosY;
	}
	ImGui::SetNextWindowPos(ImVec2(0, templatePosY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, templateH));

	ImGui::Begin("Template Info",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse);

	ImGui::Text("Fps/s: %.2f", system_->GetFPSPerSecond());

	ImGui::End();
}

void MapMakingEditor::ImGuiRightMenuBar() {
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 w1Pos{};
	ImVec2 w1Size{};

	// ===== Camera =====
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, menuBarHeight_));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
	ImGui::Begin("Camera",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	rightInspectorWidth_ = ImGui::GetWindowSize().x;

	ImGui::Text("BackTo(0,0):");
	ImGui::SameLine();
	if (ImGui::Button("Click")) {
		//debugCamera_.lock()->SetCamera(debugCamera_.lock()->GetDefaultTransform());
	}

	//ImGui::Checkbox("Use Debug Camera", &useDebugCamera);

	w1Pos = ImGui::GetWindowPos();
	w1Size = ImGui::GetWindowSize();

	ImGui::End();


	// ===== Map Info =====
	float window2Y = w1Pos.y + w1Size.y;
	ImVec2 w2Pos;
	ImVec2 w2Size;

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window2Y));
	if (selectedMap_ != -1) {
		ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
		ImGui::Begin("Map Generator Info",
			nullptr,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_AlwaysAutoResize);

		if (selectedMap_ != -1) {
			MapInfo& selectedMapInfo = *mapData_[selectedMap_];
			ImGui::Text("Selected Map: %s", selectedMapInfo.tileMapData.name.c_str());
			int sizeX = (selectedMapInfo.tileMapData.Row.empty()) ? 0 : (int)selectedMapInfo.tileMapData.Row[0].size();
			int sizeY = (selectedMapInfo.tileMapData.Row.empty()) ? 0 : (int)selectedMapInfo.tileMapData.Row.size();
			ImGui::Text("Map Size: %d x %d", sizeX, sizeY);
		}

		w2Pos = ImGui::GetWindowPos();
		w2Size = ImGui::GetWindowSize();
	} else {
		ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
		ImGui::Begin("Map Generator Info",
			nullptr,
			ImGuiWindowFlags_NoMove);

		w2Pos = ImGui::GetWindowPos();
		w2Size = ImGui::GetWindowSize();
	}

	ImGui::End();

	// ===== Map Info =====
	float window3Y = w2Pos.y + w2Size.y;
	ImVec2 w3Pos;
	ImVec2 w3Size;

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window3Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
	ImGui::Begin("Map2",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Dummy");

	w3Pos = ImGui::GetWindowPos();
	w3Size = ImGui::GetWindowSize();

	ImGui::End();


	// ===== Inspector Details =====
	float window4Y = w3Pos.y + w3Size.y;
	float mainBottomY = io.DisplaySize.y;
	float secondHeight = mainBottomY - window4Y;
	if (secondHeight < 0) secondHeight = 0;

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window4Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, secondHeight));
	ImGui::Begin("Variable",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Dummy");

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

	// 下のウィンドウの高さ
	int lineCount = 15; // 下のウィンドウに表示する
	if (lineCount < 0) lineCount = 0;
	float bottomH;
	float bottomWindowY;
	if (isMTWindowOpen_) {
		bottomH = lineH * lineCount + titleH;
		if (isMBWindowOpen_) bottomWindowY = io.DisplaySize.y - lineH * lineCount - titleH;
		else bottomWindowY = io.DisplaySize.y - titleH;
	} else {
		bottomH = io.DisplaySize.y - menuBarHeight_ - titleH;
		bottomWindowY = menuBarHeight_ + titleH;
	}

	// 中間視窗高度
	float centerTotalH;
	if (isMBWindowOpen_)centerTotalH = io.DisplaySize.y - menuBarHeight_ - bottomH;
	else centerTotalH = io.DisplaySize.y - menuBarHeight_ - menuBarHeight_;

	// ===== Top Window（跟 Maps / Template Info 一樣的寫法）=====
	ImGui::SetNextWindowPos(ImVec2(centerPosX, centerPosY));
	ImGui::SetNextWindowSize(ImVec2(centerWidth, centerTotalH));

	isMTWindowOpen_ = ImGui::Begin("Map",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);


	ImGui::End();

	// ===== Bottom Window =====
	ImGui::SetNextWindowPos(ImVec2(centerPosX, bottomWindowY));
	ImGui::SetNextWindowSize(ImVec2(centerWidth, bottomH));

	isMBWindowOpen_ = ImGui::Begin("Inspector Details",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	InputBigTextString("Codeing Text", codeingText_);
	ImGui::Button("Apply");

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
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
		}
		if (isButtonClicked) {
			showSaveWindow_ = false;
		}
		ImGui::End();
	}
}

void MapMakingEditor::ImGuiNewWindow() {
	if (showNewWindow_) {
		float lineHeight = ImGui::GetTextLineHeightWithSpacing();
		float padding = ImGui::GetStyle().WindowPadding.y * 2;
		float titleBar = ImGui::GetFrameHeight();
		float fixedHeight = lineHeight * 5 + padding + titleBar;
		ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));
		ImGui::SetNextWindowFocus();
		ImGui::Begin("New Map", &showNewWindow_, ImGuiWindowFlags_NoResize);

		/// セーブパース
		InputTextString("Save File Path", saveFilePath_);
		/// 名前
		InputTextString("Map Name", newMapName_);

		/// サイズ
		ImGui::Text("Size(x, y):");
		ImGui::SameLine();
		ImGui::InputInt2("##Size", &newMapSize_[0]);

		bool isButtonClicked = false;
		if (ImGui::Button("New")) {

			// Add Map 動作
			auto newMap = std::make_unique<MapInfo>();

			newMap->tileMapData = TileMapData{};
			newMap->tileMapData.name = newMapName_.c_str();
			for (int y = 0; y < newMapSize_[1]; ++y) {
				for (int x = 0; x < newMapSize_[0]; ++x) {
					newMap->tileMapData.Column.push_back(0);
				}
				newMap->tileMapData.ColumnPush();
			}

			newMap->isSaved = false;
			newMap->savePath = saveFilePath_;
			mapData_.push_back(std::move(newMap));

			if (newMapName_ == "NewMap" + std::to_string(newCounter_)) {
				newCounter_++;
			}

			isButtonClicked = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
		}
		if (isButtonClicked) {
			showNewWindow_ = false;
		}
		ImGui::End();
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
#endif

/// ImDrawList* draw = ImGui::GetWindowDrawList();
/// ImVec2 p = ImGui::GetCursorScreenPos();
/// draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 100, p.y + 50), IM_COL32(0, 255, 0, 255));