#pragma once
#include <string>
#include "Utility/EnumNameFinder/EnumNameFinder.h"

enum WindowMode {
	FULLSCREEN,				// フルスクリーン
	WINDOWED_Fixed,			// 大小固定、コマンドによって切り替える(要切り替えがいる
	WINDOWED_Dynamic,		// 切り替え自由
	WINDOWED_Scale,			// 固定比率で切り替え
	Num_Of_Type,			// 総数
};

enum gamepadType {
	NODEVICE,
	DIRECTINPUT,
	XINPUT
};

/// ============ エンジン内部準備したモデル ============ ///
enum class EngineModelType {
	None,
	Plane,
	Triangle,
	Cube,
	Sphere,
	Ring,
	Cylinder,
	NumOfEngineModel
};
template <>
inline const std::map<EngineModelType, std::string> EnumNameMap<EngineModelType> = {
	{ EngineModelType::Plane,    "Plane" },
	{ EngineModelType::Triangle, "Triangle" },
	{ EngineModelType::Cube,     "Cube" },
	{ EngineModelType::Sphere,   "Sphere" },
	{ EngineModelType::Ring,     "Ring" },
	{ EngineModelType::Cylinder, "Cylinder" },
};
/// 内蔵モデルを作る時の数値（初期値 = エンジン起動時に作られるデフォルトモデルと同じ）
struct EngineModelParams {
	int   sphereLat = 10;
	int   sphereLong = 10;
	int   ringDivide = 32;
	float ringOuter = 1.0f;
	float ringInner = 0.5f;
	int   cylinderDivide = 32;
	float cylinderTop = 1.0f;
	float cylinderBottom = 1.0f;
	float cylinderHeight = 3.0f;
	bool  cylinderReverseY = false;
};
/// ============================================= ///


class config
{
private:
	/// window設定
	static const int kDefaultClientWidth_ = 1280;   /// デフォルトのクライアント幅
	static const int kDefaultClientHeight_ = 720;   /// デフォルトのクライアント高さ
	static const std::string kDefaultClientTitle_;  /// デフォルトのクライアントタイトル
	static const WindowMode kDefaultWindowMode_ = WINDOWED_Fixed;  /// デフォルトのウィンドウモード

	/// 描画設定
	static const int kDebugLineNumInstance_ = 5000;   /// DebugLineの最大インスタンス数
	static const int k2DTileNumInstance_ = 50000;     /// Tileの最大インスタンス数
	static const int k3DTileNumInstance_ = 10000;     /// Tileの最大インスタンス数
	static const int kParticleCPUNumInstance_ = 50000;  /// Particleの最大インスタンス数

	/// 最大マテリアル数設定
	static const int kMaxMaterialNum_ = 100000;        /// 最大マテリアル数
	static const int kMaxMaterialLayerNum_ = 3;       /// 最大マテリアルレイヤー数(MaterialForGPUには手動更新)

	/// 最大Offset数設定
	static const int kMaxOffsetNum_ = 10000;           /// 最大Offset数

	/// 最大SRV数(最大テキスチャ枚数)
	static const int kMaxSRVNum_ = 512;               /// 最大SRV数(最大テキスチャ枚数)

	/// 最大ライト数設定
	static const int kMaxLightNum_ = 50;              /// 最大ライト数

	/// 後処理のレイヤー数設定
	static const int kPostProcessLayer = 5;

	/// 最大ログエントリ数
	static const int kMaxEntries_ = 2000;

private:

	/// ============================= mesh関連 ============================= ///
	/// Sphereの分割数
	static const int kDefaultSphereDivideNum_ = 18;			/// Sphereの分割数

private:

	/// ========================= 現在のウィンドウ関連 ========================= ///
	static WindowMode clientWindowMode_;
	static std::string clientTitle_;
	static int clientWidth_;
	static int clientHeight_;

	static std::string psoCacheFolderPath_;           /// PSOキャッシュの保存先
	static std::string psoCacheFolderName_;           /// PSOキャッシュの保存先

	static bool isConnectKeyboard_;
	static bool isConnectMouse_;
	static bool isConnectGamePad_;
	static gamepadType gamePadType_;
	static int XInputDriverIP_;

public:
	/// ====================== 各エンジン生成モデルの最大数 ======================== ///
	// デバッグライン
	static int GetDebugLineNumInstance() { return kDebugLineNumInstance_; }
	// 2Dマーシュ
	static int Get2DTileNumInstance() { return k2DTileNumInstance_; }
	// 3Dマーシュ
	static int Get3DTileNumInstance() { return k3DTileNumInstance_; }
	// パーティクル
	static int GetParticleCPUNumInstance() { return kParticleCPUNumInstance_; }
	// マテリアル最大数数
	static int GetMaxMaterialNum() { return kMaxMaterialNum_; }
	// 
	static int GetMaxMaterialLayerNum() { return kMaxMaterialLayerNum_; }
	//
	static int GetMaxOffsetNum() { return kMaxOffsetNum_; }
	//
	static int GetMaxSRVNum() { return kMaxSRVNum_; }
	//
	static int GetMaxLightNum() { return kMaxLightNum_; }
	//
	static int GetPostProcessLayer() { return kPostProcessLayer; }
	//
	static int GetDefaultSphereDivideNum() { return kDefaultSphereDivideNum_; }


	/// ============================= Window関連 ============================= ///
	// ------------------------------- const設定 ------------------------------ //
	// 画面のモード関連(Set/Getter)
	static WindowMode GetDefaultWindowMode() { return kDefaultWindowMode_; }

	// タイトル関連(Set/Getter)
	static const std::string GetDefaultTitle() { return kDefaultClientTitle_; }

	// 画面の横幅(Set/Getter)
	static int GetDefaultWidth() { return kDefaultClientWidth_; }

	// 画面の縦幅(Set/Getter)
	static int GetDefaultHeight() { return kDefaultClientHeight_; }

	// 最大セーブログエントリ数
	static int GetMaxEntries() { return kMaxEntries_; }
	
	// ------------------------------- 現在の設定 ------------------------------ //
	// 画面のモード関連(Set/Getter)
	static WindowMode GetClientWindowMode() { return clientWindowMode_; }
	static void SetClientWindowMode(WindowMode mode);

	// タイトル関連(Set/Getter)
	static void SaveClientTitle(const std::string& clientTitle) { clientTitle_ = clientTitle; };
	static const std::string GetClientTitle() { return clientTitle_; }

	// 画面の横幅(Set/Getter)
	static void SaveClientWidth(int clientWidth) { clientWidth_ = clientWidth; }
	static int GetClientWidth() { return clientWidth_; }

	// 画面の縦幅(Set/Getter)
	static void SaveClientHeight(int clientHeight) { clientHeight_ = clientHeight; }
	static int GetClientHeight() { return clientHeight_; }

	/// ============================== PSO関連 =============================== ///
	static void SavePsoCacheFolderPath(const std::string& path) { psoCacheFolderPath_ = path; }
	static std::string GetPsoCacheFolderPath() { return psoCacheFolderPath_; }
	static void SavePsoCacheFolderName(const std::string& name) { psoCacheFolderName_ = name; }
	static std::string GetPsoCacheFolderName() { return psoCacheFolderName_; }

	/// ============================= 入力関連 ================================ ///
	static void SetConnectKeyboard(bool* state) { isConnectKeyboard_ = *state; }
	static bool GetKeyboardState() { return isConnectKeyboard_; }
	static void SetConnectMouse(bool* state) { isConnectMouse_ = *state; }
	static bool GetMouseState() { return isConnectMouse_; }
	static void SetConnectGamePad(bool* state) { isConnectGamePad_ = *state; }
	static bool GetGamePadState() { return isConnectGamePad_; }
	static void SetGamePadType(gamepadType Type) { gamePadType_ = Type; }
	static gamepadType GetGamePadType() { return gamePadType_; }
	static void SetXInputIp(int IP) { XInputDriverIP_ = IP; }
	static int GetXInputIp() { return XInputDriverIP_; }

public:
	static int default_White5x5_TextureHandle_;

public:
	/// デフォルトエンジン生成メッシュバッファハンドル
	static int default_Plane_MeshBufferHandle_;
	static int default_Triangle_MeshBufferHandle_;
	static int default_Sprite2D_MeshBufferHandle_;
	static int default_Cube_MeshBufferHandle_;
	static int default_Sphere_MeshBufferHandle_;
	static int default_SkyCube_MeshBufferHandle_;
	static int default_Ring_MeshBufferHandle_;
	static int default_Cylinder_MeshBufferHandle_;

public:
	/// デフォルトPSOハンドル
	static int default_LightModel_;
	static int default_RenderModelTypes_;
	static int default_RasterizerMode_;
	static int default_BlendMode_;
	static int default_DepthStenctilState_;
	static int default_RenderTargetFormatType_;
};

