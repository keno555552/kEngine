#include "MapMakingEditor.h"
#include "DebugDraw.h"
#include "ImguiManager.h"


MapMakingEditor::MapMakingEditor(kEngine* system) {
	system_ = system;
	debugText_ = "";
	auto& roomMap = currentMapInfo_.tileMapData;
}

MapMakingEditor::~MapMakingEditor() {}

void MapMakingEditor::Update() {
	DeletePart();
}

void MapMakingEditor::Draw() {
	ImGuiPart();
}

#ifdef USE_IMGUI
void MapMakingEditor::ImGuiPart() {

	ImGuiIO& io = ImGui::GetIO();
	menuBarHeight_ = ImGui::GetFrameHeight();

	//ImGui::ShowDemoWindow();

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
	ImGuiDoubleCheckWindow();
	ImGuiMapNewWindow();
	ImGuiNewVariablesWindow();
}


void MapMakingEditor::DeletePart() {
	if (!isDelete_) return;
	switch (selectMode_) {
	case SelectModeForMME::TileMap:
		// Handle tile map deletion
		break;
	case SelectModeForMME::MapMaker:
		if (currentMakerIndex_ >= 0 && currentMakerIndex_ < static_cast<int>(mapMakerlist_.size())) {
			mapMakerlist_.erase(mapMakerlist_.begin() + currentMakerIndex_);
		} else {
			debugText_ += "Error: Invalid currentMakerIndex_ for delete.\n";
		}
		break;
	case SelectModeForMME::Variable:
	{
		// MapMaker index 安全檢查
		if (currentMakerIndex_ < 0 || currentMakerIndex_ >= mapMakerlist_.size()) {
			debugText_ += "Error: Invalid currentMakerIndex_ for variable delete.\n";
			break;
		}
		auto& list = mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList;
		// Variable index 安全檢查
		if (currentVariableIndex_ < 0 || currentVariableIndex_ >= list.size()) {
			debugText_ += "Error: Invalid currentVariableIndex_ for delete.\n";
			break;
		}
		// 執行刪除
		list.erase(list.begin() + currentVariableIndex_);
		// 刪除後修正 index
		if (currentVariableIndex_ >= list.size())
			currentVariableIndex_ = (int)list.size() - 1;

		if (list.empty())
			currentVariableIndex_ = -1;
		break;
	}

	default:
		debugText_ += "Error: No Selection for delete.\n";
		break;
	}
	isDelete_ = false;
}

void MapMakingEditor::MapInfoLoadJson() {
	if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {
		auto& maker = mapMakerlist_[currentMakerIndex_];
		currentMapInfo_.jsonData = maker.GetMapMakerJson();
	}
}

void MapMakingEditor::ImGuiLeftMenuBar() {

	ImGuiIO& io = ImGui::GetIO();

	float selectorH = menuBarHeight_;
	float lineH = ImGui::GetTextLineHeightWithSpacing();
	float titleH = ImGui::GetFrameHeight();

	float templateLines = 1;
	float templateH = templateLines * lineH + 20.0f + titleH;

	float selectorPosY = menuBarHeight_;
	float templatePosY = io.DisplaySize.y - templateH;

	ImVec2 w1Pos{};
	ImVec2 w1Size{};

	/// =========================
	/// 1. Mode Selector
	/// =========================
	ImGui::SetNextWindowPos(ImVec2(0, selectorPosY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, selectorH));

	ImGui::Begin("##LeftSelector",
		nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse);

	if (ImGui::Selectable("MakerEditor", modeEditing_ == 0, 0, ImVec2(80, selectorH - 4)))
		modeEditing_ = 0;
	ImGui::SameLine();
	if (ImGui::Selectable("RoomEditor", modeEditing_ == 1, 0, ImVec2(80, selectorH - 4)))
		modeEditing_ = 1;
	ImGui::SameLine();
	if (ImGui::Selectable("MapGenerator", modeEditing_ == 2, 0, ImVec2(80, selectorH - 4)))
		modeEditing_ = 2;
	ImGui::SameLine();

	w1Pos = ImGui::GetWindowPos();
	w1Size = ImGui::GetWindowSize();

	ImGui::End();

	/// =========================
	/// 2. Combo MapData \ MapMaker（補償用選擇器）
	/// =========================
	float window2Y = w1Pos.y + w1Size.y;
	ImVec2 w2Pos;
	ImVec2 w2Size;

	ImGui::SetNextWindowPos(ImVec2(0, window2Y));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, 0)); // AutoResize

	isMapDataWindowOpen_ = ImGui::Begin("##LeftDataSelector",
		nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::SetNextItemWidth(leftInspectorWidth_ - 20);

	/// ------------------------------------------------------------
	/// 如果 Tag 選的是 MapData → 這裡顯示 MapMaker 的 list
	/// ------------------------------------------------------------
	if (selectModeForLeftData_ == SelectModeForLeftData::MapData) {
		// 目前選擇的 MapMaker 名稱
		const char* preview = "None";
		if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size())
			preview = mapMakerlist_[currentMakerIndex_].GetMapName().c_str();

		if (ImGui::BeginCombo("##MakerSelector", preview)) {
			for (int i = 0; i < mapMakerlist_.size(); i++) {
				auto& maker = mapMakerlist_[i];
				bool selected = (currentMakerIndex_ == i);

				if (ImGui::Selectable(maker.GetMapName().c_str(), selected)) {
					currentMakerIndex_ = i;
				}
			}
			ImGui::EndCombo();
		}
	}

	/// ------------------------------------------------------------
	/// 如果 Tag 選的是 MapMaker → 這裡顯示 MapData 的 list
	/// ------------------------------------------------------------
	else if (selectModeForLeftData_ == SelectModeForLeftData::MapMaker) {
		const char* preview = "None";
		if (currentMapDataIndex_ >= 0 && currentMapDataIndex_ < mapDataList_.size())
			preview = mapDataList_[currentMapDataIndex_].dataName.c_str();

		if (ImGui::BeginCombo("##DataSelector", preview)) {
			for (int i = 0; i < mapDataList_.size(); i++) {
				auto& data = mapDataList_[i];
				bool selected = (currentMapDataIndex_ == i);

				if (ImGui::Selectable(data.dataName.c_str(), selected)) {
					currentMapDataIndex_ = i;
					// 同樣不改 selectModeForLeftData_
					// 因為 Tag 決定下方視窗顯示 MapMaker
				}
			}
			ImGui::EndCombo();
		}
	}

	w2Pos = ImGui::GetWindowPos();
	w2Size = ImGui::GetWindowSize();

	ImGui::End();


	/// =========================
	/// Tag 控制視窗（無標題）
	/// =========================

	ImVec2 wTPos;
	ImVec2 wTSize;

	ImGui::SetNextWindowPos(ImVec2(0, w2Pos.y + w2Size.y));   // 放在 Mode Selector 下方
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, 0));

	ImGui::Begin("##TagController",
		nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoCollapse);

	if (ImGui::BeginTabBar("LeftTagBar")) {
		if (ImGui::BeginTabItem("MapMaker")) {
			selectModeForLeftData_ = SelectModeForLeftData::MapMaker;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("MapData")) {
			selectModeForLeftData_ = SelectModeForLeftData::MapData;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	wTPos = ImGui::GetWindowPos();
	wTSize = ImGui::GetWindowSize();

	ImGui::End();


	ImVec2 w4Pos;
	ImVec2 w4Size;

	//float colimenH = (io.DisplaySize.y - wTSize.y - selectorH - templateH - titleH * 2);
	float colimenH = (io.DisplaySize.y - templateH - wTPos.y - wTSize.y);

	if (selectModeForLeftData_ == SelectModeForLeftData::MapData) {
		ImGuiMapDataWindow(wTPos, &w4Pos, wTSize, &w4Size, colimenH);
	} else if (selectModeForLeftData_ == SelectModeForLeftData::MapMaker) {
		ImGuiMapMakerWindow(wTPos, &w4Pos, wTSize, &w4Size, colimenH);
	}


	// =========================
	// 4. Template Info（獨立視窗）
	// =========================

	if (isMapDataSelected_ == 0 && isMakerSelected_ == 0 && isVariableSelected_ == 0) {
		templatePosY = w4Pos.y + w4Size.y;
		templateH = io.DisplaySize.y - templatePosY;
	}
	ImGui::SetNextWindowPos(ImVec2(0, templatePosY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, templateH));

	ImGui::Begin("Engine Info",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse);

	ImGui::Text("Fps/s: %.2f", system_->GetFPSPerSecond());

	ImGui::End();
}


void MapMakingEditor::ImGuiMapDataWindow(ImVec2 size, ImVec2* outputSize, ImVec2 pos, ImVec2* outputPos, float colimenH) {

	float windowY = size.y + pos.y;
	ImVec2 wSize;
	ImVec2 wPos;

	float oneColimenH = colimenH / 3;

	float titleH = ImGui::GetFrameHeight();

	ImGui::SetNextWindowPos(ImVec2(0, windowY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, oneColimenH));

	isMapMakerWindowOpen_ = ImGui::Begin("MapData List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);


	float full2W = leftInspectorWidth_ - 20;
	float buttonSize2 = full2W / 5.0f;
	float add2W = buttonSize2;
	float dou2W = buttonSize2;
	float sav2W = buttonSize2;
	float del2W = buttonSize2;


	if (ImGui::Button("New", ImVec2(add2W, 20))) {
		MapMaker newMapMakerData;
		std::string newName = "new_MapMakerData";
		if (newMapMakerDataCounter_ != 0) newName += "_" + std::to_string(newMapMakerDataCounter_);
		newMapMakerData.SetMapName(newName);
		newMapMakerDataCounter_++;
		mapMakerlist_.push_back(newMapMakerData);
	}

	ImGui::SameLine();

	/// Double
	if (ImGui::Button("Double", ImVec2(dou2W, 20))) {
		if (isMakerSelected_) {
			MapMaker newMapMakerData = mapMakerlist_[currentMakerIndex_];
			newMapMakerData.SetMapName(newMapMakerData.GetMapName() + "_copy");
			mapMakerlist_.push_back(newMapMakerData);
		} else {
			debugText_ += "Error: No generator selected for doubling.\n";
		}
	}

	ImGui::SameLine();

	/// Save
	if (ImGui::Button("Save", ImVec2(sav2W, 20))) {
		if (isMakerSelected_) {

			auto& maker = mapMakerlist_[currentMakerIndex_];

			maker.SetCodingText(codeingText_);
			maker.LoadScript(codeingText_);

			maker.Compile();

			maker.BuildJson();

			if (maker.ExportMapMakerToJson(maker.GetProjectPath())) {
				maker.SetSaved(true);
				debugText_ += "Save: MapBuilder.json exported.\n";
			} else {
				debugText_ += "Save Error: Cannot write MapBuilder.json.\n";
				debugText_ += maker.GetDebugText();
			}
		} else {
			debugText_ += "Error: No generator selected for save.\n";
		}
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(del2W, 20))) {
		doubleCheck_ = true;
		selectMode_ = SelectModeForMME::MapMaker;
	}

	ImGui::PopStyleColor(3);

	ImGui::BeginChild("GeneratorList", ImVec2(0, ImGui::GetContentRegionAvail().y), true);
	for (int i = 0; i < mapMakerlist_.size(); i++) {
		auto& Data = mapMakerlist_[i]; // Assuming each MapMakerData has at least one prototype
		std::string label = Data.GetMapName() + (Data.isSaved() ? "" : " *");

		if (ImGui::Selectable(label.c_str(), currentMakerIndex_ == i)) {

			if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {
				mapMakerlist_[currentMakerIndex_].SetCodingText(codeingText_);
			}

			currentMakerIndex_ = i;
			selectMode_ = SelectModeForMME::MapMaker;

			// 今回選択されたGeneratorのCodingTextを取得
			codeingText_ = mapMakerlist_[i].GetCodingText();
		}
	}
	ImGui::EndChild();

	if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) isMakerSelected_ = true;
	else isMakerSelected_ = false;

	wPos = ImGui::GetWindowPos();
	wSize = ImGui::GetWindowSize();

	ImGui::End();


	// =========================
	// 4. Variable List（獨立視窗）
	// =========================

	float window2Y = wPos.y + wSize.y;

	ImGui::SetNextWindowPos(ImVec2(0, window2Y));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, oneColimenH * 2));

	isVariablesWindowOpen_ = ImGui::Begin("Variables List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float full3W = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float buttonSize3 = full3W / 5.0f;
	float add3W = buttonSize3;
	float dou3W = buttonSize3;
	float sav3W = buttonSize3;
	float del3W = buttonSize3;


	if (ImGui::Button("New", ImVec2(add3W, 20))) {
		if (isMakerSelected_) {
			showNewVariablesWindow_ = true;
		} else {
			debugText_ += "Error: No generator selected for adding new variable.\n";
		}
	}

	ImGui::SameLine();

	/// Double
	if (ImGui::Button("Double", ImVec2(dou3W, 20))) {
		if (isMakerSelected_) {
			MapMaker newMapMakerData = mapMakerlist_[currentMakerIndex_];
			newMapMakerData.SetMapName(newMapMakerData.GetMapName() + "_copy");
			mapMakerlist_.push_back(newMapMakerData);
		} else {
			debugText_ += "Error: No generator selected for doubling.\n";
		}
	}

	ImGui::SameLine();

	/// Save
	if (ImGui::Button("Save", ImVec2(sav3W, 20))) {
		if (isMakerSelected_) {

			auto& maker = mapMakerlist_[currentMakerIndex_];

			maker.SetCodingText(codeingText_);
			maker.LoadScript(codeingText_);

			maker.Compile();

			maker.BuildJson();

			if (maker.ExportMapMakerToJson(maker.GetProjectPath())) {
				maker.SetSaved(true);
				debugText_ += "Save: MapBuilder.json exported.\n";
			} else {
				debugText_ += "Save Error: Cannot write MapBuilder.json.\n";
				debugText_ += maker.GetDebugText();
			}
		} else {
			debugText_ += "Error: No generator selected for save.\n";
		}
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(del3W, 20))) {
		doubleCheck_ = true;
		selectMode_ = SelectModeForMME::Variable;
	}

	ImGui::PopStyleColor(3);

	ImGui::BeginChild("VariableList", ImVec2(0, titleH + ImGui::GetFrameHeight() + 20), true);
	int index[2] = { 0, 0 };

	ImGui::EndChild();


	*outputPos = ImGui::GetWindowPos();
	*outputSize = ImGui::GetWindowSize();

	ImGui::End();
}

void MapMakingEditor::ImGuiMapMakerWindow(ImVec2 size, ImVec2* outputSize, ImVec2 pos, ImVec2* outputPos, float colimenH) {

	float windowY = size.y + pos.y;
	ImVec2 wSize;
	ImVec2 wPos;

	float oneColimenH = colimenH / 3;

	float titleH = ImGui::GetFrameHeight();

	ImGui::SetNextWindowPos(ImVec2(0, windowY));
	ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, oneColimenH));

	isMapMakerWindowOpen_ = ImGui::Begin("MapMaker List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);


	float fullW = leftInspectorWidth_ - 20;
	float buttonSize = fullW / 5.0f;
	float addW = buttonSize;
	float douW = buttonSize;
	float savW = buttonSize;
	float delW = buttonSize;


	if (ImGui::Button("New", ImVec2(addW, 20))) {
		MapMaker newMapMakerData;
		std::string newName = "new_MapMakerData";
		if (newMapMakerDataCounter_ != 0) newName += "_" + std::to_string(newMapMakerDataCounter_);
		newMapMakerData.SetMapName(newName);
		newMapMakerDataCounter_++;
		mapMakerlist_.push_back(newMapMakerData);
	}

	ImGui::SameLine();

	/// Double
	if (ImGui::Button("Double", ImVec2(douW, 20))) {
		if (isMakerSelected_) {
			MapMaker newMapMakerData = mapMakerlist_[currentMakerIndex_];
			newMapMakerData.SetMapName(newMapMakerData.GetMapName() + "_copy");
			mapMakerlist_.push_back(newMapMakerData);
		} else {
			debugText_ += "Error: No generator selected for doubling.\n";
		}
	}

	ImGui::SameLine();

	/// Save
	if (ImGui::Button("Save", ImVec2(savW, 20))) {
		if (isMakerSelected_) {

			auto& maker = mapMakerlist_[currentMakerIndex_];

			maker.SetCodingText(codeingText_);
			maker.LoadScript(codeingText_);

			maker.Compile();

			maker.BuildJson();

			if (maker.ExportMapMakerToJson(maker.GetProjectPath())) {
				maker.SetSaved(true);
				debugText_ += "Save: MapBuilder.json exported.\n";
			} else {
				debugText_ += "Save Error: Cannot write MapBuilder.json.\n";
				debugText_ += maker.GetDebugText();
			}
		} else {
			debugText_ += "Error: No generator selected for save.\n";
		}
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(delW, 20))) {
		doubleCheck_ = true;
		selectMode_ = SelectModeForMME::MapMaker;
	}

	ImGui::PopStyleColor(3);

	ImGui::BeginChild("GeneratorList", ImVec2(0, ImGui::GetContentRegionAvail().y), true);
	for (int i = 0; i < mapMakerlist_.size(); i++) {
		auto& Data = mapMakerlist_[i]; // Assuming each MapMakerData has at least one prototype
		std::string label = Data.GetMapName() + (Data.isSaved() ? "" : " *");

		if (ImGui::Selectable(label.c_str(), currentMakerIndex_ == i)) {

			if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {
				mapMakerlist_[currentMakerIndex_].SetCodingText(codeingText_);
			}

			currentMakerIndex_ = i;
			selectMode_ = SelectModeForMME::MapMaker;

			// 今回選択されたGeneratorのCodingTextを取得
			codeingText_ = mapMakerlist_[i].GetCodingText();
		}
	}
	ImGui::EndChild();

	if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) isMakerSelected_ = true;
	else isMakerSelected_ = false;

	wPos = ImGui::GetWindowPos();
	wSize = ImGui::GetWindowSize();

	ImGui::End();


	// =========================
	// Variable List
	// =========================

	float window2Y = wPos.y + wSize.y;

	ImGui::SetNextWindowPos(ImVec2(0, window2Y));
	if (isMapMakerWindowOpen_) {
		ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, oneColimenH * 2));
	} else {
		ImGui::SetNextWindowSize(ImVec2(leftInspectorWidth_, oneColimenH * 3));
	}

	isVariablesWindowOpen_ = ImGui::Begin("Variables List",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	float full2W = leftInspectorWidth_ - 20;   // 你原本的按鈕寬度邏輯
	float buttonSize2 = full2W / 5.0f;
	float add2W = buttonSize2;
	float dou2W = buttonSize2;
	float sav2W = buttonSize2;
	float del2W = buttonSize2;


	if (ImGui::Button("New", ImVec2(add2W, 20))) {
		if (isMakerSelected_) {
			showNewVariablesWindow_ = true;
		} else {
			debugText_ += "Error: No generator selected for adding new variable.\n";
		}
	}

	ImGui::SameLine();

	/// Double (Duplicate Variable)
	if (ImGui::Button("Double", ImVec2(dou2W, 20))) {

		if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {

			auto& list = mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList;

			if (currentVariableIndex_ >= 0 && currentVariableIndex_ < list.size()) {

				Variable newVar = list[currentVariableIndex_];
				newVar.name += "_copy";

				list.push_back(newVar);

				debugText_ += "Variable duplicated.\n";

			} else {
				debugText_ += "Error: No variable selected for duplication.\n";
			}

		} else {
			debugText_ += "Error: No MapMaker selected for variable duplication.\n";
		}
	}

	ImGui::SameLine();

	/// Save (Apply changes to variable)
	if (ImGui::Button("Save", ImVec2(sav2W, 20))) {
		   
		//if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {
		//
		//	auto& list = mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList;
		//
		//	if (currentVariableIndex_ >= 0 && currentVariableIndex_ < list.size()) {
		//
		//		// 你現在的 UI 已經直接修改 list[currentVariableIndex_] 的內容
		//		// 所以 Save 其實不需要做任何事，只需要提示
		//		debugText_ += "Variable saved.\n";
		//
		//	} else {
		//		debugText_ += "Error: No variable selected for save.\n";
		//	}
		//
		//} else {
		//	debugText_ += "Error: No MapMaker selected for variable save.\n";
		//}
	}



	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.10f, 0.10f, 1.0f));

	if (ImGui::Button("Delete", ImVec2(del2W, 20))) {
		doubleCheck_ = true;
		selectMode_ = SelectModeForMME::Variable;
	}

	ImGui::PopStyleColor(3);

	bool skipVariableList = false;

	ImGui::BeginChild("VariableList", ImVec2(0, ImGui::GetContentRegionAvail().y), true);

	// =========================
	// MapMaker 安全檢查
	// =========================
	if (mapMakerlist_.empty()) {
		ImGui::Text("No MapMaker available.");
		skipVariableList = true;
	}

	if (!skipVariableList) {

		// =========================
		// currentMakerIndex_ 安全管理
		// =========================
		if (currentMakerIndex_ < 0) currentMakerIndex_ = 0;
		if (currentMakerIndex_ >= mapMakerlist_.size())
			currentMakerIndex_ = (int)mapMakerlist_.size() - 1;

		auto& list = mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList;

		// =========================
		// 顯示變數列表
		// =========================
		for (int i = 0; i < list.size(); i++) {

			auto& v = list[i];
			ImGui::PushID(i);

			// -------------------------
			// Type（左側）
			// -------------------------
			ImGui::Text("Type: %s", VarTypeToString(v.type).c_str());

			// -------------------------
			// 選擇用 checkbox（右側）
			// -------------------------
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20); // 靠右
			bool selected = (currentVariableIndex_ == i);
			if (ImGui::Checkbox("##select", &selected)) {
				currentVariableIndex_ = selected ? i : -1;
			}

			// -------------------------
			// Name
			// -------------------------
			InputTextString("Name", v.name);

			// -------------------------
			// Value
			// -------------------------
			switch (v.type) {
			case VarType::Bool:
				if (auto val = std::get_if<bool>(&v.value))
					InputCheckbox("Value", *val);
				break;

			case VarType::Int:
				if (auto val = std::get_if<int>(&v.value))
					InputInt("Value", *val);
				break;

			case VarType::Float:
				if (auto val = std::get_if<float>(&v.value))
					InputFloat("Value", *val);
				break;

			case VarType::Double:
				if (auto val = std::get_if<double>(&v.value))
					InputDouble("Value", *val);
				break;

			case VarType::String:
				if (auto val = std::get_if<std::string>(&v.value))
					InputTextString("Value", *val);
				break;

			case VarType::Vector2:
				if (auto val = std::get_if<Vector2>(&v.value))
					InputVector2("Value", *val);
				break;

			case VarType::Vector3:
				if (auto val = std::get_if<Vector3>(&v.value))
					InputVector3("Value", *val);
				break;

			case VarType::Vector4:
				if (auto val = std::get_if<Vector4>(&v.value))
					InputVector4("Value", *val);
				break;
			}

			ImGui::Separator();
			ImGui::PopID();
		}
	}

	ImGui::EndChild();

	*outputPos = ImGui::GetWindowPos();
	*outputSize = ImGui::GetWindowSize();

	ImGui::End();
}







void MapMakingEditor::ImGuiRightMenuBar() {
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 w1Pos{};
	ImVec2 w1Size{};

	float selectorH = menuBarHeight_;
	float lineH = ImGui::GetTextLineHeightWithSpacing();
	float titleH = ImGui::GetFrameHeight();

	float lastLineNum = 5;
	float debugLogH = lineH * lastLineNum + 20.0f + titleH;

	float debugLogY = io.DisplaySize.y - debugLogH;

	// ===== Camera =====
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, menuBarHeight_));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
	ImGui::Begin("Camera",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	rightInspectorWidth_ = ImGui::GetWindowSize().x;

	if (ImGui::Button("BackTo(0,0)")) {
		//debugCamera_.lock()->SetCamera(debugCamera_.lock()->GetDefaultTransform());
	}

	//ImGui::Checkbox("Use Debug Camera", &useDebugCamera);

	w1Pos = ImGui::GetWindowPos();
	w1Size = ImGui::GetWindowSize();

	ImGui::End();

	// ===== Info =====
	float window2Y = w1Pos.y + w1Size.y;
	ImVec2 w2Pos;
	ImVec2 w2Size;

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window2Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));

	if (isMapDataSelected_) {
		ImGui::Begin("MapData Info",
			nullptr,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_AlwaysAutoResize);

		if (currentMapDataIndex_ >= 0 && currentMapDataIndex_ < (int)mapDataList_.size()) {
			MapData& data = mapDataList_[currentMapDataIndex_];
			ImGuiMapDataShowing(data);
		}

		w2Pos = ImGui::GetWindowPos();
		w2Size = ImGui::GetWindowSize();
	} else if (isMakerSelected_) {

		ImGui::Begin("Map Maker Info",
			nullptr,
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_AlwaysAutoResize);

		if (currentMakerIndex_ >= 0 && currentMakerIndex_ < (int)mapMakerlist_.size()) {
			MapMaker& selectedMakerInfo = mapMakerlist_[currentMakerIndex_];
			ImGuiMapMakerShowing(selectedMakerInfo);
		}

		w2Pos = ImGui::GetWindowPos();
		w2Size = ImGui::GetWindowSize();

	} else {
		ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, 0));
		ImGui::Begin("Info",
			nullptr,
			ImGuiWindowFlags_NoMove);
	}

	w2Pos = ImGui::GetWindowPos();
	w2Size = ImGui::GetWindowSize();

	ImGui::End();

	// ===== Variable Details =====
	float window3Y = w2Pos.y + w2Size.y;
	ImVec2 w3Pos;
	ImVec2 w3Size;
	w3Size.y = io.DisplaySize.y - window3Y - debugLogH - titleH;


	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window3Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, w3Size.y));
	ImGui::Begin("Variable Details",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	w3Pos = ImGui::GetWindowPos();
	w3Size = ImGui::GetWindowSize();

	if (isMakerSelected_) {
		auto& dataBase = mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList;

		for (int i = 0; i < dataBase.size(); i++) {

			Variable* v = &dataBase[i];

			ImGui::SetNextItemWidth(120.0f);
			ImGui::InputText(("##Name%s" + std::to_string(i)).c_str(), &v->name);

			ImGui::SameLine();

			ImGui::SetNextItemWidth(150.0f);
			switch (v->type) {
			case VarType::Bool:
				ImGui::Checkbox(("##Bool%s" + std::to_string(i)).c_str(), &GetValueRef<bool>(*v));
				break;
			case VarType::Int:
				ImGui::InputInt(("##Int%s" + std::to_string(i)).c_str(), &GetValueRef<int>(*v));
				break;
			case VarType::Float:
				ImGui::InputFloat(("##Float%s" + std::to_string(i)).c_str(), &GetValueRef<float>(*v));
				break;
			case VarType::Double:
				ImGui::InputDouble(("##Double%s" + std::to_string(i)).c_str(), &GetValueRef<double>(*v));
				break;
			case VarType::String:
				ImGui::InputText(("##String%s" + std::to_string(i)).c_str(), &GetValueRef<std::string>(*v));
				break;
			case VarType::Vector2:
				ImGui::InputFloat2(("##Vector2%s" + std::to_string(i)).c_str(), &GetValueRef<Vector2>(*v).x);
				break;
			case VarType::Vector3:
				ImGui::InputFloat3(("##Vector3%s" + std::to_string(i)).c_str(), &GetValueRef<Vector3>(*v).x);
				break;
			case VarType::Vector4:
				ImGui::InputFloat4(("##Vector4%s" + std::to_string(i)).c_str(), &GetValueRef<Vector4>(*v).x);
				break;
			}
		}
	}

	ImGui::End();


	// ===== Inspector Details =====
	float window4Y = w3Pos.y + w3Size.y;
	float mainBottomY = io.DisplaySize.y;
	float secondHeight = mainBottomY - window4Y;
	if (secondHeight < 0) secondHeight = 0;

	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - rightInspectorWidth_, window4Y));
	ImGui::SetNextWindowSize(ImVec2(rightInspectorWidth_, secondHeight));
	ImGui::Begin("Debug Log",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Debug Log:");
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		debugText_ = "";
	}

	ImGui::Text("%s", debugText_.c_str());

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

	if (ImGui::BeginTabBar("##MapTabs")) {

		if (ImGui::BeginTabItem("Map")) {
			if (!currentMapInfo_.tileMapData.Row.empty()) {
				ImGui::Text("%s", currentMapInfo_.tileMapData.TileMapDataToString().c_str());
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Compile")) {
			ImGui::Text("Compile Result:");

			ImGui::Text("%s", currentMapInfo_.jsonData.c_str());

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();


	// ===== Bottom Window =====
	ImGui::SetNextWindowPos(ImVec2(centerPosX, bottomWindowY));
	ImGui::SetNextWindowSize(ImVec2(centerWidth, bottomH));

	isMBWindowOpen_ = ImGui::Begin("Inspector Details",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	InputBigTextString("Codeing Text", codeingText_);

	if (ImGui::Button("Save")) {
		if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {

			auto& maker = mapMakerlist_[currentMakerIndex_];
			maker.SetCodingText(codeingText_);
			maker.LoadScript(codeingText_);

			maker.Compile();
			maker.BuildJson();
			maker.SetSaved(false);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Apply")) {
		if (currentMakerIndex_ >= 0 && currentMakerIndex_ < mapMakerlist_.size()) {

			auto& maker = mapMakerlist_[currentMakerIndex_];
			maker.SetCodingText(codeingText_);
			maker.LoadScript(codeingText_);

			maker.Compile();
			maker.BuildJson();
			MapInfoLoadJson();
			maker.SetSaved(false);

			debugText_ += "Apply: Script updated and recompiled.\n";
		} else {
			debugText_ += "Error: No valid map maker selected.\n";
		}
	}

	ImGui::SameLine();
	ImGui::Text("Result = result[y][x]");	

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
			isSave_ = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
			selectMode_ = SelectModeForMME::None;
		}
		if (isButtonClicked) {
			showSaveWindow_ = false;
		}
		ImGui::End();
	}
}

void MapMakingEditor::ImGuiDoubleCheckWindow() {
	if (doubleCheck_) {
		float lineHeight = ImGui::GetTextLineHeightWithSpacing();
		float padding = ImGui::GetStyle().WindowPadding.y * 2;
		float titleBar = ImGui::GetFrameHeight();
		float fixedHeight = lineHeight * 3 + padding + titleBar;
		ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));
		ImGui::SetNextWindowFocus();
		ImGui::Begin("Double Check Window", &doubleCheck_, ImGuiWindowFlags_NoResize);

		ImGui::Text("Are you sure you want to delete this item?");

		bool isButtonClicked = false;
		if (ImGui::Button("Delete")) {
			isButtonClicked = true;
			isDelete_ = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
			selectMode_ = SelectModeForMME::None;
		}
		if (isButtonClicked) {
			doubleCheck_ = false;
		}
		ImGui::End();
	}
}

void MapMakingEditor::ImGuiMapNewWindow() {

	/// 最後の時だけOpenPopupをする、ついでに名前を初期化する
	if (showNewWindow_) {
		ImGui::OpenPopup("New Map");

		std::string newNumber = std::to_string(newVariableCounter_);
		if (newVariableCounter_ == 0)newNumber = "";
		newName_ = "NewMap" + newNumber;

		showNewWindow_ = false;
	}

	/// ウィンドウのサイズを計算、固定する
	float lineHeight = ImGui::GetTextLineHeightWithSpacing();
	float padding = ImGui::GetStyle().WindowPadding.y * 2;
	float titleBar = ImGui::GetFrameHeight();
	float fixedHeight = lineHeight * 5 + padding + titleBar;
	ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));

	if (ImGui::BeginPopupModal("New Map", NULL, ImGuiWindowFlags_NoResize)) {

		/// セーブパース
		InputTextString("Save File Path", saveFilePath_);
		/// 名前
		InputTextString("Map Name", newName_);

		/// サイズ

		/// サイズ
		ImGui::Text("Size(x, y):");
		ImGui::SameLine();
		ImGui::InputInt2("##Size", &newMapSize_[0]);

		bool isButtonClicked = false;
		if (ImGui::Button("New")) {

			// Add Map 動作
			MapInfo newMap{};

			newMap.tileMapData = TileMapData{};
			newMap.tileMapData.name = newName_;
			for (int y = 0; y < newMapSize_[1]; ++y) {
				for (int x = 0; x < newMapSize_[0]; ++x) {
					newMap.tileMapData.Column.push_back(0);
				}
				newMap.tileMapData.ColumnPush();
			}

			newMap.isSaved = false;
			newMap.savePath = saveFilePath_;
			mapList_.push_back(newMap);


			std::string newNumber = std::to_string(newVariableCounter_);
			if (newVariableCounter_ == 0)newNumber = "";
			if (newName_ == "NewMap" + newNumber) {
				newMapCounter_++;
			}

			isButtonClicked = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
		}
		if (isButtonClicked) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void MapMakingEditor::ImGuiMapDataShowing(MapData& data) {

	ImGui::Text("Selected Map: %s", data.dataName.c_str());
	InputTextString("Map Name", data.dataName);
	InputInt("Max Size X", data.xSize);
	InputInt("Max Size Y", data.ySize);

	ImGui::Button("Add Unit", ImVec2(-1, 0));
	for (const auto& unit : data.unitLists) {

	}

}

void MapMakingEditor::ImGuiMapMakerShowing(MapMaker& maker) {

	std::string name = maker.GetMapName();
	InputTextString("Map Name", name);
	maker.SetMapName(name);

	std::string savePath = maker.GetProjectPath();
	InputTextString("Save Path", savePath);
	maker.SetProjectPath(savePath);
	ImGui::Text("VariablesNumber:%d", static_cast<int>(maker.GetGeneratorVariables().variableList.size()));

}

void MapMakingEditor::ImGuiNewVariablesWindow() {
	/// 最後の時だけOpenPopupをする、ついでに名前を初期化する
	if (showNewVariablesWindow_) {
		ImGui::OpenPopup("New Variables");

		std::string newNumber = std::to_string(newVariableCounter_);
		if (newVariableCounter_ == 0)newNumber = "";
		newName_ = "NewVariable" + newNumber;

		showNewVariablesWindow_ = false;
	}

	/// ウィンドウのサイズを計算、固定する
	float lineHeight = ImGui::GetTextLineHeightWithSpacing();
	float padding = ImGui::GetStyle().WindowPadding.y * 2;
	float titleBar = ImGui::GetFrameHeight();
	float fixedHeight = lineHeight * 5 + padding + titleBar;
	ImGui::SetNextWindowSize(ImVec2(0, fixedHeight));

	/// Popup Window開始
	if (ImGui::BeginPopupModal("New Variables", NULL,
		ImGuiWindowFlags_NoResize)) {


		/// 名前
		InputTextString("Variables Name", newName_);

		/// タイプ
		ImGui::Text("Type:");
		ImGui::SameLine();
		const char* types[] = {
			"Bool\0", "Int\0", "Float\0", "Double\0",
			"String\0", "Vector2\0", "Vector3\0", "Vector4\0"
		};
		ImGui::Combo("##VariableType", &variableType_, types, IM_ARRAYSIZE(types));


		switch (variableType_) {
		case (int)VarType::Bool:
			InputCheckbox("Bool Value", newBoolValue_);
			break;
		case (int)VarType::Int:
			InputInt("Int Value", newIntValue_);
			break;
		case (int)VarType::Float:
			InputFloat("Float Value", newFloatValue_);
			break;
		case (int)VarType::Double:
			InputDouble("Double Value", newDoubleValue_);
			break;
		case (int)VarType::String:
			InputTextString("String Value", newStringValue_);
			break;
		case (int)VarType::Vector2:
			InputVector2("Vector2 Value", newVector2Value_);
			break;
		case (int)VarType::Vector3:
			InputVector3("Vector3 Value", newVector3Value_);
			break;
		case (int)VarType::Vector4:
			InputVector4("Vector4 Value", newVector4Value_);
			break;
		}


		bool isButtonClicked = false;
		if (ImGui::Button("New")) {

			if (currentMakerIndex_ != -1) {
				// Add VariableUnit 動作
				Variable newVariable{};

				// 初始化 value
				switch ((VarType)variableType_) {
				case VarType::Bool: newVariable = CreateVariable(newName_, newBoolValue_);			break;
				case VarType::Int: newVariable = CreateVariable(newName_, newIntValue_);			break;
				case VarType::Float: newVariable = CreateVariable(newName_, newFloatValue_);			break;
				case VarType::Double: newVariable = CreateVariable(newName_, newDoubleValue_);		break;
				case VarType::String: newVariable = CreateVariable(newName_, newStringValue_);		break;
				case VarType::Vector2: newVariable = CreateVariable(newName_, newVector2Value_);		break;
				case VarType::Vector3: newVariable = CreateVariable(newName_, newVector3Value_);		break;
				case VarType::Vector4: newVariable = CreateVariable(newName_, newVector4Value_);		break;
				}

				/// [選択したGenerator][変数の種類]に新しい変数を追加する
				mapMakerlist_[currentMakerIndex_].GetGeneratorVariables().variableList.push_back(newVariable);

				std::string newNumber = std::to_string(newVariableCounter_);
				if (newVariableCounter_ == 0)newNumber = "";
				if (newName_ == "NewVariable" + newNumber) {
					newVariableCounter_++;
				}
				isButtonClicked = true;
			} else {
				/// 後でエラーウィンドウに変わる
				Logger::Log("MapMakingEditor: No generator selected. Cannot add new variable.");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			isButtonClicked = true;
		}
		if (isButtonClicked) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
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

void MapMakingEditor::InputCheckbox(const char* label, bool& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::Checkbox(newLabel.c_str(), &value);
}

void MapMakingEditor::InputInt(const char* label, int& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputInt(newLabel.c_str(), &value);
}

void MapMakingEditor::InputFloat(const char* label, float& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat(newLabel.c_str(), &value);
}

void MapMakingEditor::InputDouble(const char* label, double& value) {
	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputDouble(newLabel.c_str(), &value);
}

void MapMakingEditor::InputVector2(const char* label, Vector2& vec) {

	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat2(newLabel.c_str(), &vec.x);
}

void MapMakingEditor::InputVector3(const char* label, Vector3& vec) {

	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat3(newLabel.c_str(), &vec.x);
}

void MapMakingEditor::InputVector4(const char* label, Vector4& vec) {

	ImGui::Text("%s:", label);
	ImGui::SameLine();
	std::string newLabel = std::string("##") + label;
	ImGui::InputFloat4(newLabel.c_str(), &vec.x);
}

#endif

/// ImDrawList* draw = ImGui::GetWindowDrawList();
/// ImVec2 p = ImGui::GetCursorScreenPos();
/// draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 100, p.y + 50), IM_COL32(0, 255, 0, 255));