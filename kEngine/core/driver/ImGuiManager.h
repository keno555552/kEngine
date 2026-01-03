#pragma once

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_stdlib.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


class ImGuiManager {
public:
    static bool HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    static void Initialize(HWND hwnd, ID3D12Device* device, ID3D12CommandQueue* queue, ID3D12DescriptorHeap* srvHeap, D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle, D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle);
    static void BeginFrame();
    static void EndFrame(ID3D12GraphicsCommandList* commandList);
    static void Shutdown();
};

