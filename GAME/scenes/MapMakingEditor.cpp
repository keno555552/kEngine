#include "MapMakingEditor.h"
#include "DebugDraw.h"
#include "ImguiManager.h"

MapMakingEditor::MapMakingEditor(kEngine *system)
{
	system_ = system;
}

MapMakingEditor::~MapMakingEditor() {}

void MapMakingEditor::Update()
{
	// Implementation for update logic
}

void MapMakingEditor::Draw()
{
	ImGuiPart();
}

void MapMakingEditor::ImGuiPart()
{

	ImGuiIO &io = ImGui::GetIO();
	menuBarHeight_ = ImGui::GetFrameHeight();

	/// メニューバー
	if (ImGui::BeginMainMenuBar())
	{

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
			};
			if (ImGui::MenuItem("Load"))
			{
				showLoadWindow_ = true;
			}
			if (ImGui::MenuItem("Save"))
			{
				showSaveWindow_ = true;
			}
			if (ImGui::MenuItem("Exit"))
			{
				outcome_ = SceneOutcome::EXIT;
				isSceneEnd_ = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	/// 右側のウィンドウ群
	ImVec2 w1Pos{};
	ImVec2 w1Size{};
	{
		/// デバッグウィンドウ
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - inspectorWidth_, menuBarHeight_));
		ImGui::SetNextWindowSize(ImVec2(inspectorWidth_, 0)); // 高さを0に設定して自動調整
		ImGui::Begin("Camera",
					 nullptr,
					 ImGuiWindowFlags_NoMove |
						 ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_AlwaysAutoResize);
		// 更新寬度（使用するウィンドウの幅を取得して、インスペクタの幅を更新）
		inspectorWidth_ = ImGui::GetWindowSize().x;
		ImGui::Text("BackTo(0,0):");
		ImGui::SameLine();
		if (ImGui::Button("Click"))
		{
			// 按鈕事件
		}
		// 記錄 Inspector 的位置與大小
		w1Pos = ImGui::GetWindowPos();
		w1Size = ImGui::GetWindowSize();
		ImGui::End();
	}

	float window2Y = w1Pos.y + w1Size.y;
	ImVec2 w2Pos;
	ImVec2 w2Size;

	{
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - inspectorWidth_, window2Y));
		ImGui::SetNextWindowSize(ImVec2(inspectorWidth_, 0)); // 高度自動
		ImGui::Begin("Window 2",
					 nullptr,
					 ImGuiWindowFlags_NoMove |
						 ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Second window content");
		ImGui::Text("Item 1");
		ImGui::Text("Item 2");
		ImGui::Text("Item 3");

		w2Pos = ImGui::GetWindowPos();
		w2Size = ImGui::GetWindowSize();

		ImGui::End();
	}

	float window3Y = w2Pos.y + w2Size.y;
	float mainBottomY = io.DisplaySize.y;
	float secondHeight = mainBottomY - window3Y;

	if (secondHeight < 0)
		secondHeight = 0;

	{
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - inspectorWidth_, window3Y));
		ImGui::SetNextWindowSize(ImVec2(inspectorWidth_, secondHeight)); // 高度自動

		ImGui::Begin("Inspector Details",
					 nullptr,
					 ImGuiWindowFlags_NoMove |
						 ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_AlwaysAutoResize);
		InputBigTextString("Codeing Text", codeingText_);

		ImGui::End();
	}

	/// Popup Window
	ImGuiLoadWindow();
	ImGuiSaveWindow();
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

// --- 專用 Callback ---
static int InputTextCallback(ImGuiInputTextCallbackData *data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		auto *str = reinterpret_cast<std::string *>(data->UserData);
		str->resize(data->BufTextLen);
		data->Buf = const_cast<char *>(str->c_str());
	}
	return 0;
}

bool MapMakingEditor::InputTextString(const char *label, std::string &str, ImGuiInputTextFlags flags)
{
	flags |= ImGuiInputTextFlags_CallbackResize;

	ImGui::Text("%s:", label);
	ImGui::SameLine();

	std::string newLabel = std::string("##") + label; // 隐藏标签

	if (str.capacity() == 0)
		str.reserve(16);

	return ImGui::InputText(
		newLabel.c_str(),
		const_cast<char *>(str.c_str()),
		str.capacity() + 1,
		flags,
		InputTextCallback,
		&str);
}

bool MapMakingEditor::InputBigTextString(const char *label, std::string &str, ImGuiInputTextFlags flags)
{
	flags |= ImGuiInputTextFlags_CallbackResize;

	ImGui::Text("%s:", label);

	std::string newLabel = std::string("##") + label; // 隐藏标签

	if (str.capacity() == 0)
		str.reserve(16);

	// 取得視窗內容區域大小（扣掉 Text 的高度）
    ImVec2 avail = ImGui::GetContentRegionAvail();

	// 下方 padding
	avail.y -= ImGui::GetTextLineHeight();

	// 字一個分の左右 padding を追加
	float padX = ImGui::GetFontSize();  
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padX, ImGui::GetStyle().FramePadding.y));

	bool result = ImGui::InputTextMultiline(
		newLabel.c_str(),
		const_cast<char *>(str.c_str()),
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

/// ImDrawList* draw = ImGui::GetWindowDrawList();
/// ImVec2 p = ImGui::GetCursorScreenPos();
/// draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 100, p.y + 50), IM_COL32(0, 255, 0, 255));