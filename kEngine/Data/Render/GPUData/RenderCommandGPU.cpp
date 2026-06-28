#include "RenderCommandGPU.h"
#include "Config.h"

void ConvertRenderCommandToGPU(const RenderCommand& cpu, RenderCommandGPU& gpu) {

	/// ========== ColorGuard =========== ///
	if (gpu.guardColor[0] != cpu.guardColor[0]) gpu.guardColor[0] = cpu.guardColor[0];
	if (gpu.guardColor[1] != cpu.guardColor[1]) gpu.guardColor[1] = cpu.guardColor[1];
	if (gpu.guardColor[2] != cpu.guardColor[2]) gpu.guardColor[2] = cpu.guardColor[2];
	if (gpu.guardAmount != cpu.guardAmount)	gpu.guardAmount = cpu.guardAmount;

	/// ========== Vignette =========== ///
	if (gpu.vignetteCenter[0] != cpu.vignetteCenter.x) gpu.vignetteCenter[0] = cpu.vignetteCenter.x;
	if (gpu.vignetteCenter[1] != cpu.vignetteCenter.y) gpu.vignetteCenter[1] = cpu.vignetteCenter.y;
	if (gpu.vignetteRadius != cpu.vignetteRadius) gpu.vignetteRadius = cpu.vignetteRadius;
	if (gpu.vignetteSoftness != cpu.vignetteSoftness) gpu.vignetteSoftness = cpu.vignetteSoftness;
	if (gpu.vignetteIntensity != cpu.vignetteIntensity) gpu.vignetteIntensity = cpu.vignetteIntensity;
	if (gpu.vignetteColor[0] != cpu.vignetteColor.x) gpu.vignetteColor[0] = cpu.vignetteColor.x;
	if (gpu.vignetteColor[1] != cpu.vignetteColor.y) gpu.vignetteColor[1] = cpu.vignetteColor.y;
	if (gpu.vignetteColor[2] != cpu.vignetteColor.z) gpu.vignetteColor[2] = cpu.vignetteColor.z;
	if (gpu.vignetteColor[3] != cpu.vignetteColor.w) gpu.vignetteColor[3] = cpu.vignetteColor.w;

	/// ========== Blur =========== ///
	int cpuBlurType = static_cast<int>(cpu.blurType);
	if (cpu.blurType == KernelType::BlurBox) {
		if (gpu.blurType != cpuBlurType) gpu.blurType = cpuBlurType;
		if (gpu.blurKernelSize != cpu.blurKernelSize) gpu.blurKernelSize = cpu.blurKernelSize;
	} else if (cpu.blurType == KernelType::BlurRadial) {

		if (gpu.blurType != cpuBlurType) gpu.blurType = cpuBlurType;
		Vector2 cpuRadialCenterNormalized = { cpu.blurRadialCenter.x / static_cast<float>(config::GetClientWidth()),
											  cpu.blurRadialCenter.y / static_cast<float>(config::GetClientHeight()) };
		if (gpu.blurRadialCenter != cpuRadialCenterNormalized) {
			gpu.blurRadialCenter.x = cpuRadialCenterNormalized.x;
			gpu.blurRadialCenter.y = cpuRadialCenterNormalized.y;
		}
		if (gpu.blurRadialStrength != cpu.blurRadialStrength) gpu.blurRadialStrength = cpu.blurRadialStrength;
		if (gpu.blurRadialSampleSize != cpu.blurRadialSampleSize) gpu.blurRadialSampleSize = cpu.blurRadialSampleSize;

	} else if (cpu.blurType == KernelType::BlurCustom) {
		/// kernelSizeを計算する
		int maxIndex = -1;
		for (int y = 6; y >= 0; --y) {
			bool isData = false;
			for (int x = 6; x >= 0; --x) {
				if (cpu.blurKernelArray[y][x] != 0.0f) {
					maxIndex = std::max(x, y);
					isData = true;
				}
			}
			if (isData) break;
		}

		// snap 成 3/5/7
		int size = 3;
		if (maxIndex >= 6) size = 7;
		else if (maxIndex >= 4) size = 5;
		else if (maxIndex < 0) size = -1; // データなし

		// 更新GPU
		if (gpu.blurType != cpuBlurType) gpu.blurType = cpuBlurType;
		if (gpu.blurKernelSize != size) gpu.blurKernelSize = size;
	}

	/// ========== Outline =========== ///
	if (IsOutlineCheck(cpu)) {
		int cpuOutlineType = static_cast<int>(cpu.outlineType);
		if (gpu.outlineType != cpuOutlineType) gpu.outlineType = cpuOutlineType;
		if (gpu.outlineKernelSize != cpu.outlineKernelSize) gpu.outlineKernelSize = cpu.outlineKernelSize;
		if (gpu.outlineDepthThreshold != cpu.outlineDepthThreshold) gpu.outlineDepthThreshold = cpu.outlineDepthThreshold;
		if (gpu.outlineColor != cpu.outlineColor) gpu.outlineColor = cpu.outlineColor;
	}

	/// ============ Dissolve用コマンド ============== ///
	if (gpu.dissolveEdgeColor != cpu.dissolveEdgeColor) gpu.dissolveEdgeColor = cpu.dissolveEdgeColor;
	if (gpu.dissolveThreshold != cpu.dissolveThreshold) gpu.dissolveThreshold = cpu.dissolveThreshold;
	if (gpu.dissolveEdgeWidth != cpu.dissolveEdgeWidth) gpu.dissolveEdgeWidth = cpu.dissolveEdgeWidth;

	/// =========== RandomNoise用コマンド ============= ///
	if(cpu.randomNoiseType != RandomNoiseType::NONE)
	{
		if (gpu.randomNoiseType != static_cast<int>(cpu.randomNoiseType))
			gpu.randomNoiseType = static_cast<int>(cpu.randomNoiseType);
		if (gpu.randomNoiseAmount != cpu.randomNoiseAmount) gpu.randomNoiseAmount = cpu.randomNoiseAmount;
		if (gpu.randomNoiseTime != cpu.randomNoiseTime) gpu.randomNoiseTime = cpu.randomNoiseTime;
	}
}

bool IsBlurCheck(const RenderCommand& cpu) {
	bool isBlur = false;
	if (cpu.blurType == KernelType::BlurBox ||
		cpu.blurType == KernelType::BlurRadial ||
		cpu.blurType == KernelType::BlurCustom
		) {
		isBlur = true;
	}
	return isBlur;
}

bool IsOutlineCheck(const RenderCommand& cpu) {
	bool isOutline = false;
	if (cpu.outlineType == KernelType::OutLineLaplacian ||
		cpu.outlineType == KernelType::OutLinePrewitt ||
		cpu.outlineType == KernelType::OutLinePrewittDepth ||
		cpu.outlineType == KernelType::OutLineRoberts ||
		cpu.outlineType == KernelType::OutLineSobel ||
		cpu.outlineType == KernelType::OutLineThick) {
		isOutline = true;
	}
	return isOutline;
}
