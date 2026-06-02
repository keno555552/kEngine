#include "RenderCommand.h"

std::array<std::array<float, 7>, 7> MakeBoxBlur(int kernelSize) {
	std::array<std::array<float, 7>, 7> kernel{};
	if (kernelSize < 3 || kernelSize > 7) {
		Logger::Log("[kEngine] MakeBoxBlur() kernelSize must be at least 3 and Less than 7");
		return kernel;
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

	return kernel;
}

std::array<std::array<float, 7>, 7> MakeGaussianBlur(int kernelSize, float sigma) {
	std::array<std::array<float, 7>, 7> kernel{};
	if (kernelSize < 3 || kernelSize > 7) {
		Logger::Log("[kEngine] MakeGaussianBlur() kernelSize must be at least 3 and Less than 7");
		return kernel;
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

	return kernel;
}
