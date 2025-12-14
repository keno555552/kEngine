#pragma once

///DirectX12
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinAPI.h"
#include "Logger.h"

// Input
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#include <XInput.h>
#pragma comment(lib, "XInput.lib")

#include <GameInput.h>

/// Log
//#include <string>///<<<<<<<<formatに包含してる
#include <format>

#include <vector>
#include "LightModelType.h"
#include "config.h"

#include <dxgidebug.h>

#pragma region Input
bool CheakXInputDeviceConnected();
#pragma endregion




class DirectXCore {
public:
	DirectXCore();
	~DirectXCore();

	/// Getter
	ID3D12Device* GetDevice() { return device; };
	HWND GetHWND() { return winAPI_->GetHWND(); };
	ID3D12DescriptorHeap* GetSrvDescriptorHeap() { return srvDescriptorHeap; };
	ID3D12DescriptorHeap* GetDsvDescriptorHeap() { return dsvDescriptorHeap; };
	ID3D12DescriptorHeap* GetRtvDescriptorHeap() { return rtvDescriptorHeap; };
	uint32_t GetDesriptorSizeSRV();
	uint32_t GetDesriptorSizeRTV();
	uint32_t GetDesriptorSizeDSV();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	ID3D12GraphicsCommandList* GetCommandList() { return commandList; }

	bool isDirectInputCreated();
	IDirectInput8* GetDirectInput() { return directInput; }
	IDirectInputDevice8* GetDirectInputKeyboard() { return keyBoardDevice; }
	IDirectInputDevice8* GetDirectInputMouse() { return mouseDevice; }
	IDirectInputDevice8* GetDirectInputGamepad() { return gamepadDevice; }
	void SetDirectXInputKeyBoard();
	void SetDirectXInputMouse();
	void SetDirectXInputGamepad();
	void ReleaseDirectXInputKeyBoard();
	void ReleaseDirectXInputMouse();
	void ReleaseDirectXInputGamepad();

	/// <summary>
	/// システム初期化
	/// </summary>
	virtual void InitializeDrive(const char* kClientTitle, int kClientWidth, int kClientHeight);
	
	/// <summary>
	/// システム完結のメッセージ
	/// </summary>
	bool ProcessMessage();

	void Finalize();
protected:
	enum InputType {
		Keyboard,
		Mouse,
		GamePad
	};

	ID3D12Debug1* debugController;
	IDXGIFactory7* dxgiFactory;
	IDXGIAdapter4* useAdapter;
	ID3D12Device* device;
	ID3D12CommandQueue* commandQueue;
	ID3D12CommandAllocator* commandAllocator;
	ID3D12GraphicsCommandList* commandList;
	IDXGISwapChain4* SwapChain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	ID3D12Resource* swapChainResources[2];
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	ID3D12DescriptorHeap* rtvDescriptorHeap;
	ID3D12DescriptorHeap* srvDescriptorHeap;
	ID3D12DescriptorHeap* dsvDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	ID3D12Fence* fence;
	uint64_t fenceValue;
	HANDLE fenceEvent;
	WinAPI* winAPI_;

	/// DirectInput
	IDirectInput8* directInput;
	IDirectInputDevice8* keyBoardDevice;
	IDirectInputDevice8* mouseDevice;
	IDirectInputDevice8* gamepadDevice;



//protected:
public:

	/// 創建設備
	ID3D12Device* CreateDevice(IDXGIAdapter4* adapter);
	void InitializeGameInput();
	/// 
	
	/// DirectXInput初期化
	void InitializeDirectXInput();
	/// InputDrive作り
	void SetDirectXInput(InputType type, IDirectInputDevice8*& drive);
	void SetXInput();
	///創建命令隊列
	ID3D12CommandQueue* CreateCommandQueue();
	///創建命令分配器
	ID3D12CommandAllocator* CreateCommandAllocator();
	///創建命令隊列
	ID3D12GraphicsCommandList* CreateCommandList(ID3D12CommandAllocator* commandAllocator);
	///創建命令列表
	IDXGISwapChain4* CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* commandQueue, HWND hwnd, UINT width, UINT height);
	///創建描述符堆
	ID3D12DescriptorHeap* CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	///創建渲染目標視圖
	void CreateRenderTargetViews(IDXGISwapChain4* swapChain, ID3D12DescriptorHeap* rtvDescriptorHeap);
	///
	ID3D12Fence* CreateFence();

};


