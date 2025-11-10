#pragma once
#include "DircetXCore.h"
#include "FrameRateLimiter.h"

class DircetXController :public DirectXCore
{
public:
	~DircetXController();
	void StartFrame();
	void EndFrame();
private:
	D3D12_RESOURCE_BARRIER barrier{};
};


