#pragma once
#include <functional> 
#include "ImGuiManager.h"
#include "Utility/EnumNameFinder/EnumNameFinder.h"

class Transform;
class MaterialConfig;

/// エンジン共通の ImGui 部品。どのエディタからでも呼べる
/// 戻り値：このフレームで値が変更されたら true
namespace ImGuiWidgets {

	enum class FileSlotAction { None, Load, Delete, Extra };

	/// 表示：  Label [Load] filename [Delete]
	/// currentPath : 今のファイル（空なら "(none)"、Delete も出ない）
	/// outPath     : Load でファイルが選ばれた時、"./..." 形式のパスが入る
	FileSlotAction FileSlot(
		const char* label,
		const std::string& currentPath,
		const char* filter,
		std::string& outPath,
		const char* extraButton = nullptr);

	enum class ListButton { None, New, Double, Save, Load, Delete };

	/// [New][Double][Save][Load][Delete]（Delete は赤）
	/// 1個の幅 = fullWidth / slots（今の /6.0f と同じ見た目）
	/// 戻り値：このフレームで押されたボタン
	ListButton ListToolbar(float fullWidth, float slots = 6.0f);

	/// EnumInfo<E> が特殊化されている enum class なら何でも Combo にできる
	template <typename E>
	bool EnumCombo(const char* label, E& value) {
		const auto& nameMap = EnumNameMap<E>;
		const std::string preview = GetEnumName(value);

		bool changed = false;
		if (ImGui::BeginCombo(label, preview.c_str())) {
			for (const auto& [item, name] : nameMap) {
				const bool selected = (item == value);
				if (ImGui::Selectable(name.c_str(), selected)) {
					value = item;
					changed = true;
				}
				if (selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		return changed;
	}

	bool TransformEditor(Transform& t, bool showReset = true);
	bool UVTransformEditor(MaterialConfig& m);
	bool MaterialConfigEditor(MaterialConfig& m, bool showReset = true, bool showTexture = true);

	/// ============== Editor Layout ============== ///

	/// buildLayout の中では DockBuilderSplitNode / DockBuilderDockWindow だけ書けばいい
	/// （RemoveNode・AddNode・SetNodeSize・Finish は EditorDockSpace 側でやる）
	/// ※ DockBuilder を使うので、呼ぶ側の cpp でも imgui_internal.h が必要
	using DockLayoutBuilder = std::function<void(ImGuiID dockId, const ImVec2& size)>;

	/// メニューバーを除いた全体に DockSpace を張る（中央は透明、中央にはドック不可）
	/// imgui.ini に配置が無い（初回）か isResetLayout が true のときだけ buildLayout を呼ぶ
	void EditorDockSpace(const char* dockName, bool& isResetLayout, const DockLayoutBuilder& buildLayout);

	/// よく使う形：左にパネル1枚、残りは透明。幅は境界線ドラッグで調整（imgui.ini に保存）
	void EditorDockSpaceLeftPanel(const char* dockName, const char* leftWindowName,
		bool& isResetLayout, float leftRatio = 0.22f);
}