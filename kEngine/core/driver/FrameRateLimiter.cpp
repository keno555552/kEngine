#include "FrameRateLimiter.h"

FrameRateLimiter::FrameRateLimiter(double targetFPS) {
	targetFrameTime = 1.0f / targetFPS;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&lastTime);
}

void FrameRateLimiter::Wait() {
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	double elapsed = static_cast<double>(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
	double remaining = targetFrameTime - elapsed;

	if (remaining > 0.001f) {
		DWORD sleepMs = static_cast<DWORD>((remaining - 0.001f) * 1000.0f);
		if (sleepMs > 0) {
			Sleep(sleepMs);
		}
	}

	// busy wait
	do {
		QueryPerformanceCounter(&currentTime);
		elapsed = static_cast<double>(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
	} while (elapsed < targetFrameTime);

	// 更新為實際時間點
	lastTime = currentTime;
}