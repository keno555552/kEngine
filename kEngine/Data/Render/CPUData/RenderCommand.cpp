#include "RenderCommand.h"

void MakeBoxBlur(RenderCommand& cmd, int kernelSize) {
	std::array<std::array<float, 7>, 7> kernel{};
	if (kernelSize < 3 || kernelSize > 7) {
		Logger::Log("[kEngine] MakeBoxBlur() kernelSize must be at least 3 and Less than 7");
		return;
	}

	// 強制奇數
	int usingSize = (kernelSize % 2 == 0) ? (kernelSize + 1) : kernelSize;
	int half = usingSize / 2;

	// 総数
	int count = usingSize * usingSize;

	// 生成 Box kernel
	for (int y = 0; y < usingSize; y++) {
		for (int x = 0; x < usingSize; x++) {
			kernel[y][x] = 1.0f / count;
		}
	}
	cmd.blurType = KernelType::BlurBox;
	cmd.blurKernelSize = usingSize;
	cmd.blurKernelArray = kernel;
}

void MakeRadialBlur(RenderCommand& cmd, Vector2 center, float strength, int kernelSize) {
	cmd.blurType = KernelType::BlurRadial;
	cmd.blurRadialCenter = center;
	cmd.blurRadialStrength = strength;
	cmd.blurRadialSampleSize = kernelSize;
}

void MakeGaussianBlur(RenderCommand& cmd, int kernelSize, float sigma) {
	std::array<std::array<float, 7>, 7> kernel{};
	if (kernelSize < 3 || kernelSize > 7) {
		Logger::Log("[kEngine] MakeGaussianBlur() kernelSize must be at least 3 and Less than 7");
		return;
	}

	// 強制奇數
	int usingSize = (kernelSize % 2 == 0) ? (kernelSize + 1) : kernelSize;
	int half = usingSize / 2;

	float sum = 0.0f;

	// 生成 Gaussian kernel
	for (int y = 0; y < usingSize; y++) {
		for (int x = 0; x < usingSize; x++) {

			int offsetX = x - half;  // 中心點偏移
			int offsetY = y - half;

			kernel[y][x] = gauss((float)offsetX, (float)offsetY, sigma);
			sum += kernel[y][x];
		}
	}

	// normalize
	for (int y = 0; y < usingSize; y++) {
		for (int x = 0; x < usingSize; x++) {
			kernel[y][x] /= sum;
		}
	}

	cmd.blurType = KernelType::BlurCustom;
	cmd.blurKernelSize = usingSize;
	cmd.blurKernelArray = kernel;
}

void MakeOutlineSobel(RenderCommand& cmd) {
	std::array<std::array<float, 7>, 7> k{};
	k[0][0] = -1; k[0][1] = 0; k[0][2] = 1;
	k[1][0] = -2; k[1][1] = 0; k[1][2] = 2;
	k[2][0] = -1; k[2][1] = 0; k[2][2] = 1;
	cmd.outlineType = KernelType::OutLineSobel;
	cmd.outlineKernelSize = 3;
	cmd.outlineKernelArray = k;
}

void MakeOutlinePrewitt(RenderCommand& cmd) {
	cmd.outlineType = KernelType::OutLinePrewitt;
	cmd.outlineDepthThreshold = 0.01f;
	cmd.outlineKernelSize = 0;
}

void MakeOutlinePrewittDepth(RenderCommand& cmd) {
	cmd.outlineType = KernelType::OutLinePrewittDepth;
	cmd.outlineDepthThreshold = 0.01f;
	cmd.outlineKernelSize = 0;	
}

void MakeOutlineLaplacian(RenderCommand& cmd) {
	std::array<std::array<float, 7>, 7> k{};
	k[0][0] =  0; k[0][1] = -1; k[0][2] =  0;
	k[1][0] = -1; k[1][1] =  4; k[1][2] = -1;
	k[2][0] =  0; k[2][1] = -1; k[2][2] =  0;
	cmd.outlineType = KernelType::OutLineLaplacian;
	cmd.outlineKernelSize = 3;
	cmd.outlineKernelArray = k;
}

void MakeOutlineRoberts(RenderCommand& cmd) {
	std::array<std::array<float, 7>, 7> k{};
	k[0][0] = 0; k[0][1] = 1;
	k[1][0] = -1; k[1][1] = 0;
	cmd.outlineType = KernelType::OutLineRoberts;
	cmd.outlineKernelSize = 2;
	cmd.outlineKernelArray = k;
}

void MakeOutlineThick(RenderCommand& cmd,int kernelSize, float thickness) {
	std::array<std::array<float, 7>, 7> k{};
	if (kernelSize < 3 || kernelSize > 7) {
		Logger::Log("[kEngine] MakeGaussianBlur() kernelSize must be at least 3 and Less than 7");
		return;
	}

	// 強制奇數
	int usingSize = (kernelSize % 2 == 0) ? (kernelSize + 1) : kernelSize;
	int half = usingSize / 2;

	float sigma = thickness;

	float sum = 0.0f;
	for (int y = -half; y <= half; y++) {
		for (int x = -half; x <= half; x++) {
			float v = exp(-(x * x + y * y) / (2 * sigma * sigma));
			k[y + half][x + half] = v;
			sum += v;
		}
	}

	// normalize
	for (int y = 0; y < usingSize; y++)
		for (int x = 0; x < usingSize; x++)
			k[y][x] /= sum;

	cmd.outlineType = KernelType::OutLineThick;
	cmd.outlineKernelSize = usingSize;
	cmd.outlineKernelArray = k;
}

void MakeDissolve(RenderCommand& cmd, int textureIndex, float threshold, float edgeWidth, Vector3 edgeColor) {
	cmd.dissolveTextureIndex = textureIndex;
	cmd.dissolveThreshold = threshold;
	cmd.dissolveEdgeWidth = edgeWidth;
	cmd.dissolveEdgeColor = edgeColor;
}
