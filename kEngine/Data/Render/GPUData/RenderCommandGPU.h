#pragma once
#include "Data/Render/CPUData/RenderCommand.h"
#include <algorithm>

#undef max
#undef min

struct RenderCommandGPU
{
	/// =========== CopyImage用コマンド =========== ///
	/// ない
	/// ========== ColorGuard用コマンド =========== ///
	/// この色の明度を元に色を変わる
	float guardColor[3]{};
	/// guardingのAlphaみだいなもの
	float guardAmount{};
	// ------ padding counter line-------

	/// =========== Vignette用コマンド ============ ///
	/// vignetteの中心座標（0.0～1.0の範囲で、画面のどこに中心があるか）
	float vignetteCenter[2]{};
	/// vignetteの半径（0.0～1.0の範囲で、どこからvignetteが始まるか）
	float vignetteRadius{};
	/// vignetteの柔らかさ（0.0～1.0の範囲で、vignetteの境界がどれくらいぼやけるか）
	float vignetteSoftness{};
	// ------ padding counter line-------
	/// vignetteの強さ（0.0～1.0の範囲で、vignetteがどれくらい暗くなるか）
	float vignetteIntensity{};
	float padding[3]{};
	// ------ padding counter line-------
	/// vignetteの色
	float vignetteColor[4]{};
	// ------ padding counter line-------

	/// ============= Blur用コマンド ============== ///
	/// ここは毎回BlurをDraw前に、CPUDataを使ってこれを変わるように、使用するBlurCommandを選択すること
	/// Blurの種類
	int blurType{};
	/// カーネルのサイズ（例：3なら3x3のカーネルを使う）
	int blurKernelSize{};
	/// カーネルの対応index
	int blurKernelIndex{};
	float padding2{};
	// ------ padding counter line-------

	/// RadialBlur用
	/// センター座標(スグリント座標)(転換はGPU側の仕事)
	Vector2 blurRadialCenter{};
	/// 強さ(0.0～1.0の範囲で、どれくらいぼやけるか)
	float blurRadialStrength{};
	/// サンプルサイズ(px,4-64までギリギリ安全,主にshaderの演算速度に影響する)
	int blurRadialSampleSize{};

	// ------ padding counter line-------
	/// =========== OutLine用コマンド ============= ///
	/// oulineの種類
	int outlineType{};
	/// oulineのサイズ
	int outlineKernelSize{};
	/// oulineの対応index
	int outlineKernelIndex{};
	/// 深度に関わるoutlineのための閾値
	float outlineDepthThreshold{};
	// ------ padding counter line-------
	/// outlineの色
	Vector4 outlineColor{};
	// ------ padding counter line-------

	/// ============ Dissolve用コマンド ============== ///
	/// Dissolveのエッジカラー
	Vector3 dissolveEdgeColor{};
	//hlsl自動生成padding
	// ------ padding counter line-------
	/// Dissolveの閾値(0.0～1.0の範囲で、どのくらいの明度で溶解するか)
	float dissolveThreshold{};
	/// Dissolveのエッジ幅(0.0～1.0の範囲が一番いい、エッジの幅を制御)
	float dissolveEdgeWidth{};

	/// =========== RandomNoise用コマンド ============= ///
	/// RandomNoiseの種類
	int randomNoiseType{};
	/// RandomNoiseの強さ
	float randomNoiseAmount{};
	// ------ padding counter line-------
	/// RandomNoiseの時間
	float randomNoiseTime{};

};

void ConvertRenderCommandToGPU(const RenderCommand& cpu, RenderCommandGPU& gpu);
bool IsBlurCheck(const RenderCommand& cpu);
bool IsOutlineCheck(const RenderCommand& cpu);