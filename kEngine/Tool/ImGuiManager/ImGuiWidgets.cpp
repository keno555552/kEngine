#include "ImGuiWidgets.h"
#include "Transform.h"
#include "Data/Render/CPUData/MaterialConfig.h"
#include "Tool/FileManager/FileManager.h"


namespace ImGuiWidgets {

	FileSlotAction FileSlot(
		const char* label,
		const std::string& currentPath,
		const char* filter,
		std::string& outPath,
		const char* extraButton) {
		FileSlotAction action = FileSlotAction::None;
		ImGui::PushID(label);

		ImGui::TextUnformatted(label);
		ImGui::SameLine();
		if (ImGui::Button("Load")) {
			const std::string selected = FileManager::OpenFileDialog(filter);
			if (!selected.empty()) {
				outPath = FileManager::ToRelativePath(selected);
				action = FileSlotAction::Load;
			}
		}
		if (extraButton) {
			ImGui::SameLine();
			if (ImGui::Button(extraButton)) action = FileSlotAction::Extra;
		}

		ImGui::SameLine();
		if (currentPath.empty()) {
			ImGui::TextDisabled("(none)");
		} else {
			ImGui::TextUnformatted(FileManager::GetFileName(currentPath).c_str());
			ImGui::SetItemTooltip("%s", currentPath.c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton("Delete")) action = FileSlotAction::Delete;
		}

		ImGui::PopID();
		return action;
	}

	ListButton ListToolbar(float fullWidth, float slots) {
		const ImVec2 size(fullWidth / slots, 20.0f);
		ListButton pressed = ListButton::None;

		if (ImGui::Button("New", size))    pressed = ListButton::New;
		ImGui::SameLine();
		if (ImGui::Button("Double", size)) pressed = ListButton::Double;
		ImGui::SameLine();
		if (ImGui::Button("Save", size))   pressed = ListButton::Save;
		ImGui::SameLine();
		if (ImGui::Button("Load", size))   pressed = ListButton::Load;
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button,			ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered,	ImVec4(0.80f, 0.30f, 0.30f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,	ImVec4(0.40f, 0.10f, 0.10f, 1.0f));
		if (ImGui::Button("Delete", size)) pressed = ListButton::Delete;
		ImGui::PopStyleColor(3);

		return pressed;
	}

	bool TransformEditor(Transform& t, bool showReset) {
		bool changed = false;
		changed |= ImGui::DragFloat3("Translate", &t.translate.x, 0.05f);
		changed |= ImGui::DragFloat3("Rotate", &t.rotate.x, 0.01f);   // ラジアン
		changed |= ImGui::DragFloat3("Scale", &t.scale.x, 0.01f);
		if (showReset && ImGui::Button("Reset Transform")) {
			t = CreateDefaultTransform();
			changed = true;
		}
		return changed;
	}

	bool UVTransformEditor(MaterialConfig& m) {
		bool changed = false;
		changed |= ImGui::DragFloat3("UV Translate", &m.uvTranslate.x, 0.005f);
		changed |= ImGui::DragFloat3("UV Scale", &m.uvScale.x, 0.01f);
		changed |= ImGui::DragFloat("UV Rotate", &m.uvRotate.z, 0.01f);
		if (ImGui::Button("Reset UV")) {
			m.uvTranslate = { 0.0f, 0.0f, 0.0f };
			m.uvScale = { 1.0f, 1.0f, 1.0f };
			m.uvRotate = { 0.0f, 0.0f, 0.0f };
			changed = true;
		}
		return changed;
	}

	bool MaterialConfigEditor(MaterialConfig& m, bool showReset, bool showTexture) {
		bool changed = false;

		if (showTexture) {
			ImGui::SeparatorText("Texture");
			changed |= ImGui::Checkbox("Use Model Texture", &m.useModelTexture);
			changed |= ImGui::InputInt("Texture Handle", &m.textureHandle);
		}

		ImGui::SeparatorText("UV");
		changed |= UVTransformEditor(m);

		ImGui::SeparatorText("Color");
		changed |= ImGui::ColorEdit4("Color", &m.textureColor.x);
		changed |= ImGui::ColorEdit4("Color 2", &m.textureColor2.x);
		changed |= ImGui::ColorEdit4("Color 3", &m.textureColor3.x);

		ImGui::SeparatorText("PSO");
		changed |= EnumCombo("Light Model", m.lightModelType);
		changed |= EnumCombo("Render Model", m.renderModelType);
		changed |= EnumCombo("Blend Mode", m.blendModeType);
		changed |= EnumCombo("Rasterizer", m.rasterizerMode);
		changed |= EnumCombo("Depth Stencil", m.depthStencilType);

		ImGui::SeparatorText("Lighting");
		changed |= ImGui::Checkbox("Enable Lighting", &m.enableLighting);
		changed |= ImGui::DragFloat("Shininess", &m.shininess, 0.5f, 1.0f, 256.0f);
		changed |= ImGui::DragFloat("Intensity", &m.intensity, 0.01f, 0.0f, 10.0f);
		changed |= ImGui::DragFloat("Reflective", &m.reflectiveStrength, 0.01f, 0.0f, 1.0f);
		changed |= ImGui::DragFloat("Height Scale", &m.heightScale, 0.01f);

		/// uvTransformMatrix / materialResourceHandle / drawState はエンジン内部用なので出さない

		if (showReset && ImGui::Button("Reset Material")) {
			m = InitMaterialConfig();
			changed = true;
		}
		return changed;
	}
}