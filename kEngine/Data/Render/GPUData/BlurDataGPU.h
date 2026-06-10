#pragma once
#include "Data/Render/CPUData/RenderCommand.h"

struct BlurDataGPU
{
	float kernel[49]{};
	float padding[3]{};

	inline void ConvertBlurCommand(const RenderCommand& cpu) {
		for (int y = 0; y < 7; ++y) {
			for (int x = 0; x < 7; ++x) {
				kernel[y * 7 + x] = cpu.blurKernelArray[y][x];
			}
		}
	}
};
