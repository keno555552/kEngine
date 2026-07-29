#pragma once
#include <cstdint>
#include <wrl/client.h>      // Microsoft::WRL::ComPtr
#include <d3d12.h>           // ID3D12Resource, descriptor handles
#include <dxgiformat.h>      // DXGI_FORMAT
#include "Vector4.h"


struct RenderTexture {

	/// RenderTextureのリソースとビューハンドルをまとめた構造体
	uint32_t resourceRtvHandle{};
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU{};

	/// Resourceの現在の状態
	D3D12_RESOURCE_STATES currentState{};

	/// SRV用のリソースとビューハンドル
	uint32_t SrvIndex{};
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU{};
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU{};

	/// DepthStencil用のリソースとビューハンドル
	uint32_t depthResourceSrvIndex = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthResource;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU{};
	D3D12_CPU_DESCRIPTOR_HANDLE depthSrvHandleCPU{};
	D3D12_GPU_DESCRIPTOR_HANDLE depthSrvHandleGPU{};

	/// Resourceの現在の状態
	D3D12_RESOURCE_STATES depthState{};


	/// 描画する大きさ
	UINT width{};
	UINT height{};

	/// フォーマット
	DXGI_FORMAT format;

	/// クリアカラー
	Vector4 clearColor;
};

/// PingPong用のRenderTexture
struct PPRenderTexture {
	RenderTexture inputRT{};
	RenderTexture outputRT{};

	void Init();
	void Init(int width, int height, DXGI_FORMAT format, Vector4 clearColor);
	void Swap();
};
