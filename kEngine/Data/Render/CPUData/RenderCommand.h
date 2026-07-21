#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <array>
#include "LinearAlgebra/Gauss.h"
#include "Logger.h"
#include "Config.h"

/// 先行宣言
struct RenderCommand;
enum class KernelType {
	NONE,
	BlurBox,
	BlurRadial,
	BlurCustom,
	OutLineSobel,
	OutLinePrewitt,
	OutLinePrewittDepth,
	OutLineLaplacian,
	OutLineRoberts,
	OutLineThick,
	//OutLineDepth,
	//OutLineNormal,
	NumOfBlur
};

enum class RandomNoiseType {
	NONE,
	WhiteNoise,
};

struct KernelCommand
{
	KernelType kernelType = KernelType::NONE;
	int kernelSize = 5;
	float kernel[7][7]{};	/// 最大7x7のカーネルを想定している
};

#pragma region KernelMaker
/// Blurのカーネルを作る関数
void MakeBoxBlur(RenderCommand& cmd, int kernelSize);
void MakeRadialBlur(RenderCommand& cmd, Vector2 center,float strength, int kernelSize);
void MakeGaussianBlur(RenderCommand& cmd, int kernelSize, float sigma);
void MakeOutlineSobel(RenderCommand& cmd);
void MakeOutlinePrewitt(RenderCommand& cmd);
void MakeOutlinePrewittDepth(RenderCommand& cmd);
void MakeOutlineLaplacian(RenderCommand& cmd);
void MakeOutlineRoberts(RenderCommand& cmd);
void MakeOutlineThick(RenderCommand& cmd, int kernelSize, float thickness);
void MakeDissolve(RenderCommand& cmd, int textureIndex, float threshold, float edgeWidth, Vector3 edgeColor);
void MakeNoise(RenderCommand& cmd, RandomNoiseType type, float intensity, float scale);
#pragma endregion

struct RenderCommand
{
	/// =========== CopyImage用コマンド =========== ///
	/// ない
	/// ========== ColorGuard用コマンド =========== ///
	/// この色の明度を元に色を変わる
	Vector3 guardColor { 0.2125f, 0.7154f, 0.0721f };
	/// guardingのAlphaみだいなもの
	float guardAmount = 1.0f;
	/// =========== Vignette用コマンド ============ ///
	/// vignetteの中心座標（0.0～1.0の範囲で、画面のどこに中心があるか）
	Vector2 vignetteCenter{ 0.5f, 0.5f };
	/// vignetteの半径（0.0～1.0の範囲で、どこからvignetteが始まるか）
	float vignetteRadius = 1.0f;
	/// vignetteの柔らかさ（0.0～1.0の範囲で、vignetteの境界がどれくらいぼやけるか）
	float vignetteSoftness = 0.079f;
	/// vignetteの強さ
	float vignetteIntensity = 3.21f;
	/// vignetteの色
	Vector4 vignetteColor = { 0.0f, 0.0f, 0.0f, 1.0f };

	/// ============= Blur用コマンド ============== ///
	/// ここは毎回BlurをDraw前に、CPUDataを使ってこれを変わるように、使用するBlurCommandを選択すること
	/// Blurの種類
	KernelType blurType = KernelType::NONE;
	/// kernelのサイズ（例：3なら3x3のカーネルを使う）
	int blurKernelSize = 5;
	/// kernelの対応index
	int blurKernelIndex = -1;
	/// Blurのカーネル
	std::array<std::array<float, 7>, 7> blurKernelArray{};

	/// RadialBlur用
	/// センター座標(スグリント座標)(転換はGPU側の仕事)
	Vector2 blurRadialCenter{ (config::GetClientWidth()  / 2.0f) ,
						  (config::GetClientHeight() / 2.0f) };
	/// 強さ(-1.0～1.0の範囲で、どれくらいぼやけるか、-は発散、+は収束)
	float blurRadialStrength = 0.05f;
	/// サンプルサイズ(px,4-64までギリギリ安全,主にshaderの演算速度に影響する)
	int blurRadialSampleSize = 8;

	/// blurの種類によって、使用するデータが変わる
	/// もしBoxBlurにすると、後ろのblurKernelは無視されて、GPU側でボックスの数値を計算する
	/// もしCustomBlurにすると、後ろのblurKernelが使用され、加えてkernelSizeも必要になる(なければ7x7からデータがあるところまでSizeとして使う)
	/// その上でkernelIndexはPostProcessRunnerがデータのあり所により埋まる

	/// ============ OutLine用コマンド ============== ///

	/// outlineの種類
	KernelType outlineType = KernelType::NONE;
	/// outlineKernelのサイズ
	int outlineKernelSize = 0;
	/// outlineのkernelの対応index
	int outlineKernelIndex = 0;
	/// 深度に関わるoutlineのための閾値
	float outlineDepthThreshold = 0.0f;
	/// outlineの色
	Vector4 outlineColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	std::array<std::array<float, 7>, 7> outlineKernelArray{};

	/// ============ Dissolve用コマンド ============== ///
	/// Dissolveのテクスチャの対応index
	int dissolveTextureIndex = -1;
	/// Dissolveの閾値(0.0～1.0の範囲で、どのくらいの明度で溶解するか)
	float dissolveThreshold = 0.5f;
	/// Dissolveのエッジ幅(0.0～1.0の範囲が一番いい、エッジの幅を制御)(個人的に0.01が綺麗たと思う)
	float dissolveEdgeWidth = 0.01f;
	/// Dissolveのエッジカラー
	Vector3 dissolveEdgeColor{ 1.0f, 1.0f, 1.0f };


	/// =========== RandomNoise用コマンド ============= ///
	/// RandomNoiseの種類
	RandomNoiseType randomNoiseType = RandomNoiseType::NONE;
	/// RandomNoiseの強さ
	float randomNoiseAmount = 0.5f;
	/// RandomNoiseの時間
	float randomNoiseTime = 0.0f;

};


///NOTE:
/// RenderCommandはCPU側、RenderCommandGPUはGPU側の資料
/// 転換、コピーのデータはRenderCommandGPUに格納される
/// RenderCommandGPUはRenderCommand.hlslと対応するので
/// データの順番、形、padding等々を注意してください<<<<<<<<<<<<<<<重要!!!