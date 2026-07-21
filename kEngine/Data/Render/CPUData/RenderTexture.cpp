#include "RenderTexture.h"
#include "Resource/ResourceManager.h"
#include <utility>
#include "Config.h"

void PPRenderTexture::Init() {
	inputRT = ResourceManager::GetInstance()->CreateRenderTexture(
		config::GetClientWidth(),
		config::GetClientHeight(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		Vector4{ 0.1f, 0.25f, 0.5f, 1.0f });
	outputRT = ResourceManager::GetInstance()->CreateRenderTexture(
		config::GetClientWidth(),
		config::GetClientHeight(),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		Vector4{ 0.1f, 0.25f, 0.5f, 1.0f });
}

void PPRenderTexture::Init(int width, int height, DXGI_FORMAT format, Vector4 clearColor) {

	inputRT = ResourceManager::GetInstance()->CreateRenderTexture( width, height, format, clearColor);
	outputRT = ResourceManager::GetInstance()->CreateRenderTexture(width, height, format, clearColor);
}

void PPRenderTexture::Swap() {
	std::swap(inputRT, outputRT);
}
