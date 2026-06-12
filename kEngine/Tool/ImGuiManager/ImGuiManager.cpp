#include "ImGuiManager.h"
#include "DirectXController.h"
#include "DescriptorManager/SrvManager/SrvManager.h"


bool ImGuiManager::HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
}

void ImGuiManager::Initialize(DirectXController* dxComm) {
	SrvManager* srvManager = SrvManager::GetInstance();
    uint32_t srvIndex = srvManager->Allocate();
    dxComm->GetCommandQueue();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // 啟用 Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // 可選：讓視窗可以拖出主視窗

    io.Fonts->AddFontDefault();   // ★ 新版 ImGui 必須手動加字體
    ImGui::StyleColorsDark();

    //if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //    ImGuiStyle& style = ImGui::GetStyle();
    //    style.WindowRounding = 0.0f;
    //    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    //}

    ImGui_ImplWin32_Init(dxComm->GetHWND());
    ImGui_ImplDX12_Init(
        dxComm->GetDevice(),
        2,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        srvManager->GetDescriptorHeap(),
        srvManager->GetCPUDescriptorHandle(srvIndex),
        srvManager->GetGPUDescriptorHandle(srvIndex));
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

void ImGuiManager::BeginFrame(ID3D12GraphicsCommandList* commandList) {

   // ImGuiIO& io = ImGui::GetIO();

    ID3D12DescriptorHeap* heaps[] = { SrvManager::GetInstance()->GetDescriptorHeap() };
    commandList->SetDescriptorHeaps(1, heaps);

    //// ★ 在這裡建 FontTexture（GPU 已綁定 heap）
    //if (isFirstFrame_) {
    //    ImGui_ImplDX12_CreateDeviceObjects();
    //    Logger::Log("TexIsBuilt = %d\n", io.Fonts->TexIsBuilt);
    //    Logger::Log("TexID = %p\n", io.Fonts->TexID);
    //    isFirstFrame_ = false;
    //}

    //Logger::Log("ImGui BeginFrame, FrameCount = %f\n", io.DeltaTime);
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
