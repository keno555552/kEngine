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
#include <LinearAlgebra/Vector4.h>
#include <functional>

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
	EffectAdapter effectAdapter_{system_};

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

	/// パーティクルハンドル
	int TH_particleHandle_ = 0;
	int TH_particleHandle2_ = 0;

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

	float leftInspectorWidth_ = 300.0f;
	bool isEffectWindowOpen_ = true;
	bool isParticleWindowOpen_ = true;
	bool isLinkerWindowOpen_ = true;

	float rightInspectorWidth_ = 300.0f;
	float menuBarHeight_ = 0.0f;
	bool isShootNonStop_ = false;
	Timer shootTime;

	bool isMBLWindowOpen_ = false;
	bool isMBRWindowOpen_ = false;

	bool showLoadWindow_ = false;
	bool showSaveWindow_ = false;

	std::string codeingText_;

	std::string loadFilePath_;
	std::string saveFilePath_;

	/// Camera関連
	bool useDebugCamera = false;

	/// DebugData
	std::string debugText_;

	/// 共用
	std::string newName_;
	bool isPause_ = false;
	bool doubleCheck_ = false;

	SelectMode saveLoadMode_ = SelectMode::None;
	bool isSave_ = false;
	bool isLoad_ = false;
	bool isDelete_ = false;

private:
	/// ======== カメラ関連 ========= ///
	void CameraPart();


	/// ========== UI関連 ========== ///
	/// SaveLoad関連
	void SaveLoadPart();
	void SavePart();
	void LoadPart();

	/// Delete関連
	void DeletePart();

	void SetNullSavePathToDesktop();
	void SetNullLoadPathToDesktop();

#ifdef USE_IMGUI
	void ImGuiPart();

	void ImGuiLeftMenuBar();
	void ImGuiRightMenuBar();
	void ImGuiMidWindow();

	void ImGuiLoadWindow();
	void ImGuiSaveWindow();
	void ImGuiDoubleCheckWindow();

	void ImGuiEffectShowing();
	void ImGuiParticleShowing(ParticlePrototype& particle);
	void ImGuiLinkerShowing(EmitterLink& linker);

	bool InputTextString(const char *label, std::string &str, ImGuiInputTextFlags flags = 0);
	bool InputBigTextString(const char *label, std::string &str, ImGuiInputTextFlags flags = 0);
	void InputInt(const char* label, int& value);
	void InputFloat(const char* label, float& value);
	void InputVector3(const char* label, Vector3& vec);
	void InputVector4(const char* label, Vector4& vec);

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

#endif // USE_IMGUI
};
