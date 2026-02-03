#pragma once
#include "DirectXCore.h"
#include "FrameRateLimiter.h"
#pragma comment(lib,"winmm.lib")

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

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


