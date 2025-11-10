#pragma once
#include "DircetXCore.h"
#include "FrameRateLimiter.h"
#pragma comment(lib,"winmm.lib")

class DircetXController :public DirectXCore
{
public:
	DircetXController();
	~DircetXController();
	void StartFrame();
	void EndFrame();
private:
	D3D12_RESOURCE_BARRIER barrier{};
};


