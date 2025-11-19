#pragma once
#include "DirectXCore.h"
#include "FrameRateLimiter.h"
#pragma comment(lib,"winmm.lib")

class DirectXController :public DirectXCore
{
public:
	DirectXController();
	~DirectXController();
	void StartFrame();
	void EndFrame();
private:
	D3D12_RESOURCE_BARRIER barrier{};
};


