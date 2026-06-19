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
	int blurType;
	/// カーネルのサイズ（例：3なら3x3のカーネルを使う）
	int blurKernelSize;
	/// カーネルの対応index
	int blurKernelIndex;

	/// =========== OutLine用コマンド ============= ///
	/// oulineの種類
	int outlineType;
	// ------ padding counter line-------
	/// oulineのサイズ
	int outlineKernelSize;
	/// oulineの対応index
	int outlineKernelIndex;
	/// 深度に関わるoutlineのための閾値
	float outlineDepthThreshold = 0.0f;
	float padding2{};
	// ------ padding counter line-------
	/// outlineの色
	Vector4 outlineColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	// ------ padding counter line-------

};

void ConvertRenderCommandToGPU(const RenderCommand& cpu, RenderCommandGPU& gpu);
bool IsBlurCheck(const RenderCommand& cpu);
bool IsOutlineCheck(const RenderCommand& cpu);