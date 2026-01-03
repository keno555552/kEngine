#include "ImGuiManager.h"

bool ImGuiManager::HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
}

void ImGuiManager::Initialize(HWND hwnd, 
                              ID3D12Device* device, 
                              ID3D12CommandQueue* queue, 
	                          ID3D12DescriptorHeap* srvHeap,
                              D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle, 
                              D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(
        device, 
        2,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        srvHeap,
        CPUDescriptorHandle,
        GPUDescriptorHandle);
}

/// IMGUI_CHECKVERSION();
/// ImGui::CreateContext();
/// ImGui::StyleColorsDark();
/// ImGui_ImplWin32_Init(winAPI_->GetHWND());
/// ImGui_ImplDX12_Init(
///     device,
/// 	swapChainDesc.BufferCount,
/// 	rtvDesc.Format,
/// 	srvDescriptorHeap,
/// 	srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
/// 	srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

void ImGuiManager::BeginFrame() {
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::EndFrame(ID3D12GraphicsCommandList* commandList) {
    // 実際のcommandListのImGuiの描画コマンドを積む
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void ImGuiManager::Shutdown() {
    /// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
// こういうもんである。初期化と逆順に行う
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
