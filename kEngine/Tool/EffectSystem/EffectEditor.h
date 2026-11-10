#pragma once
#include "BaseScene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "Lighting\Light.h"
#include "Camera\DebugCamera.h"
#include "GAME\Object\DetailButton\DetailButton.h"
#include "GameObject\button\button.h"
#include "Tool\DefaultMenu\DefaultMenu.h"
#include "EngineAssets\Particle\HitSpark.h"
#include "EffectSystem/Adapter/EffectAdapter.h"
#include "Utility/EnumNameFinder/EnumNameFinder.h"
#include <LinearAlgebra/Vector4.h>
#include <functional>
#include "ImGuiWidgets.h"
#include "Renderer/Resource/ResourceManager.h"
#include "Renderer/Mesh/ModelGroup.h"

enum class SelectMode {
	None,
	Effect,
	Particle,
	Linker
};


class EffectEditor : public BaseScene
{
public:
	EffectEditor(kEngine *system);
	~EffectEditor();

	void Update() override;
	void Draw() override;

private:

	/// ============= システム関連 ==============///
	EffectAdapter effectAdapter_{ nullptr };
	RandomMaker randomMaker_;          // seed の自動生成用

	/// ============= ライト関連 ==============///
	std::unique_ptr<Light> light1_ = nullptr;
	std::unique_ptr<Light> light2_ = nullptr;
	std::unique_ptr<Light> light3_ = nullptr;
	std::unique_ptr<AreaLight> areaLight_ = nullptr;
	std::unique_ptr<AreaLight> areaLight2_ = nullptr;

	/// ============== カメラ関連 ==============///
	/// ステージカメラ(mainカメラ)
	std::weak_ptr<Camera> camera_;
	/// デバックカメラ
	std::weak_ptr<DebugCamera> debugCamera_;
	/// 今使っているカメラ
	std::weak_ptr<Camera> usingCamera_;

	/// =========== リソースハンドル ============///
	/// モデルハンドル
	int TH_skydomeModelHandle_ = 0;

	/// テキスチャーハンドル(TH)
	int TH_whiteTextureHandle_ = 0;
	int TH_clicleTextureHandle_ = 0;
	int TH_effectTextureHandle_ = 0;
	int TH_centerAnchorHandle_ = 0;

	/// サントラハンドル
	int TH_soundHandle_ = 0;

	/// ========== プレビュー関連 ========== ///
	int previewEffectIndex_ = -1;          // 今 ParticleManager に入っている Effect
	size_t previewParticleCount_ = 0;      // 作った時の数（増減したら作り直す）
	size_t previewLinkCount_ = 0;
	bool isPreviewDirty_ = true;           // 中身が変わった → 作り直す

	int dds = 0;

	/// ============= ゲームオブジェクト ==============///
	/// 元ある類
	std::unique_ptr<Object> skybox_ = nullptr;
	std::unique_ptr<Object> ground_ = nullptr;
	std::unique_ptr<Object> centerAnchor_ = nullptr;

	/// ========= オブジェクトリスト ==========///
	/// EffectData<EffectData, isSaved >
	std::vector <EffectData> effectDataList_;

	/// ======== 選択関連 ========== ///
	bool isEffectSelected_ = false;
	SelectMode selectMode_ = SelectMode::None;
	int currentEffectIndex_ = -1;
	int currentParticleIndex_ = -1;
	int currentLinkIndex_ = -1;

	/// ======== newCounter ========== ///
	int newEffectCounter_ = 0;
	int newParticleCounter_ = 0;
	int newLinkCounter_ = 0;


	/// ========== UI関連 ========== ///
	bool isResetLayout_ = false;

	bool isShootNonStop_ = false;
	Timer shootTime;

	std::string codeingText_;

	/// Camera関連
	bool useDebugCamera = false;

	/// DebugData
	std::string debugText_;

	/// 共用
	std::string newName_;
	int previewHandle_ = -1;
	bool isPause_ = false;
	bool doubleCheck_ = false;

	SelectMode saveLoadMode_ = SelectMode::None;
	bool isDelete_ = false;

	/// ========== 保存関連 ========== ///
	bool useRelativePath_ = true;        

	/// 保存の予約（ボタン → Update で処理）
	struct SaveRequest {
		SelectMode target = SelectMode::Effect;   // Effect / Particle / Linker
		int effectIndex = -1;
		int itemIndex = -1;                       // Particle / Linker の時だけ
		bool forceDialog = false;                 // true = Save As
	};
	std::vector<SaveRequest> saveQueue_;

	/// 上書き確認待ち
	struct PendingSave {
		bool active = false;
		SaveRequest request;
		bool detach = false;                      // Effect の Save As
		EffectSavePlan plan;                      // Effect 用
		std::string unitPath;                     // Particle / Linker 用
		std::vector<std::string> conflicts;
	};
	PendingSave pendingSave_;

private:
	/// ======== カメラ関連 ========= ///
	void CameraPart();


	/// ========== UI関連 ========== ///
	/// SaveLoad関連
	void SaveLoadPart();
	void SavePart();
	void LoadEffectFromDialog();
	void ImportUnitFromDialog(SelectMode target);

	/// 保存の予約
	void RequestSave(int effectIndex, bool forceDialog);
	void RequestSaveAll();
	/// 1つ処理：パス決定 → 計画 → 衝突があれば確認待ち、なければ書く
	void RequestSaveUnit(SelectMode target, int effectIndex, int itemIndex, bool forceDialog);
	/// 1つ処理：種類ごとに振り分け
	void BeginSave(const SaveRequest& request);
	void BeginSaveEffect(const SaveRequest& request);
	void BeginSaveUnit(const SaveRequest& request);
	/// Particle / Linker を実際に書く
	bool ExecuteSaveUnit(const SaveRequest& request, const std::string& path);
	/// 実際に書く
	bool ExecuteSave(int effectIndex, const EffectSavePlan& plan, bool detach);
	/// 計画の中で「自分のものではない既存ファイル」を探す
	std::vector<std::string> FindConflicts(const EffectData& effect, const EffectSavePlan& plan) const;
	/// ダイアログで親フォルダを選ばせて、<親>/<Effect名>/<Effect名>.json を返す（キャンセルで空）
	std::string AskEffectPath(const EffectData& effect);
	/// Effectフォルダの中にある単元の保存先を外す（共用フォルダにあるものはそのまま）
	void DetachEffectFolder(EffectData& effect);
	/// 今の選択が有効か（フラグではなく、毎回 index を直接確かめる）
	bool HasValidEffect() const {
		return currentEffectIndex_ >= 0 && currentEffectIndex_ < (int)effectDataList_.size();
	}
	void RenameParticleInLinks(EffectData& effect, const std::string& oldName, const std::string& newName);

	/// ======= プレビュー関連 ======= ///
	/// 選択中の Effect を ParticleManager に入れる（変わってなければ何もしない）
	void RefreshPreview();
	/// 選択中の Effect を発射
	void ShootPreview();

	/// ======= 外観関連（MapEditor と同じ：材質は1つ、Part0 が本体）======= ///
	/// モデルを替える：Part を作り直す（ファイルモデルは Part 数 = メッシュ数）
	void SetObjectModel(ObjectData& obj, int modelHandle, bool isFileModel);
	/// 手で選んだテクスチャを全 Part に
	void SetObjectTexture(ObjectData& obj, int textureHandle);
	/// モデル付属のテクスチャに戻す（内蔵モデルなら白）
	void UseModelTexture(ObjectData& obj);
	/// Part0 の材質を他の Part へ
	void SyncPartMaterials(ObjectData& obj);

	/// Delete関連
	void DeletePart();

#ifdef USE_IMGUI
	void ImGuiDockSpace();
	void BuildDefaultLayout(ImGuiID dockId, const ImVec2& size);
	float CalcButtonWidth(int count);

	void ImGuiPart();

	void ImGuiLeftMenuBar();
	void ImGuiRightMenuBar();
	void ImGuiMidWindow();

	void ImGuiOverwriteWindow();
	void ImGuiDoubleCheckWindow();

	bool ImGuiEffectShowing(EffectData& effect);
	bool ImGuiParticleShowing(ParticlePrototype& particle);
	bool ImGuiLinkerShowing(EmitterLink& linker);

	bool ImGuiModelShowing(ParticlePrototype& particle);
	bool ImGuiMaterialShowing(ParticlePrototype& particle);



	bool InputTextString(const char *label, std::string &str, ImGuiInputTextFlags flags = 0);
	bool InputBigTextString(const char *label, std::string &str, ImGuiInputTextFlags flags = 0);
	bool InputInt(const char* label, int& value);
	bool InputFloat(const char* label, float& value);
	bool InputVector3(const char* label, Vector3& vec);
	bool InputVector4(const char* label, Vector4& vec);

	template <typename EnumType>
	bool EnumCombo(const char* label, EnumType& value, const char* const items[], int itemCount) {
		ImGui::Text("%s", label);
		ImGui::SameLine();
		int current = static_cast<int>(value);
		if (ImGui::Combo(("##" + std::string(label)).c_str(), &current, items, itemCount)) {
			value = static_cast<EnumType>(current);
			return true;
		}
		return false;
	}

	/// EnumNameMap<T> から項目を作る Combo（LightModelType のように -1 から始まっても OK）
	/// hidden：パーティクルで使えない値を隠す
	template <typename EnumType>
	bool EnumMapCombo(const char* label, EnumType& value, std::initializer_list<EnumType> hidden = {}) {
		bool changed = false;
		if (ImGui::BeginCombo(label, GetEnumName(value).c_str())) {
			for (const auto& [e, name] : EnumNameMap<EnumType>) {
				if (std::find(hidden.begin(), hidden.end(), e) != hidden.end()) continue;
				const bool selected = (e == value);
				if (ImGui::Selectable(name.c_str(), selected)) { value = e; changed = true; }
				if (selected) ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		return changed;
	}

#endif // USE_IMGUI
};
