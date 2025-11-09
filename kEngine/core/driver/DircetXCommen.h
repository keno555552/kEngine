#pragma once
#include "DircetXBase.h"

class DircetXCommen:public DirectXBase
{
public:
	~DircetXCommen();
	void StartFrame();
	void EndFrame();
private:
	D3D12_RESOURCE_BARRIER barrier{};
};

class FrameRateLimiter {
public:
    FrameRateLimiter(double targetFPS = 60.0f);

    void Wait();

private:
    LARGE_INTEGER frequency;
    LARGE_INTEGER lastTime;
    double targetFrameTime;
};


