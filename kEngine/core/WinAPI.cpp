#include "WinAPI.h"
#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

/// NOTE: cppだけ定義にすると、この関数はWinAPI内でしか使えない。狙ってこれをやってる。
static DWORD StyleForMode(WindowMode mode) {
	switch(mode){
		case WindowMode::FULLSCREEN:
			return WS_POPUP;
		case WindowMode::WINDOWED_Fixed:
			return WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		case WindowMode::WINDOWED_Scale:
			return WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
		default:
			return WS_OVERLAPPEDWINDOW;
	}
	return 0;
}

WinAPI::~WinAPI() {}

LRESULT CALLBACK WinAPI::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	WinAPI* self = nullptr;

	if (msg == WM_NCCREATE) {
		// 作成直後：CreateWindowに渡したthisをウィンドウに覚えさせる
		auto* cs = reinterpret_cast<CREATESTRUCT*>(lparam);
		self = static_cast<WinAPI*>(cs->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
	} else {
		self = reinterpret_cast<WinAPI*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (self) {
		return self->HandleMessage(hwnd, msg, wparam, lparam);
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


bool WinAPI::Initialize(const std::string& kClientTitle, int kClientWidth, int kClientHeight, HINSTANCE hInstance) {
	className_ = ConvertString::SwitchStdStringWstring(kClientTitle);
	hInstance_ = hInstance;
	if (!RegisterWindowClass())										return false;
	if (!CreateMainWindow(kClientWidth, kClientHeight, hInstance))	return false;
	ShowWindow(hwnd_, SW_SHOW);

	// 起動時からフルスクリーンの場合は、ここでモニタ全体に広げる
	if (config::GetClientWindowMode() == FULLSCREEN) {
		ApplyWindowMode(FULLSCREEN, kClientWidth, kClientHeight);
	}
	return true;
}

void WinAPI::Finalize() const {
	//DestroyWindow(hwnd_);
	CoUninitialize();
}

bool WinAPI::ProcessMessage() {
	MSG msg{};

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT) {
			return false;
		}
	}
	return true;
}

void WinAPI::SetClientSize(int width, int height) {

	DWORD style = static_cast<DWORD>(GetWindowLongPtr(hwnd_, GWL_STYLE));
	RECT rc{ 0, 0, width, height};
	AdjustWindowRect(&rc, style, FALSE);
	SetWindowPos(
		hwnd_,
		nullptr,
		0,0,
		rc.right - rc.left,
		rc.bottom - rc.top,
		SWP_NOMOVE | SWP_NOZORDER
	);

}

void WinAPI::ApplyWindowMode(WindowMode mode, int clientW, int clientH) {

	if (mode == FULLSCREEN) {
		if (!isFullscreen_) {
			prevPlacement_.length = sizeof(prevPlacement_);
			GetWindowPlacement(hwnd_, &prevPlacement_);   // 元の位置を覚えておく
		}
		MONITORINFO mi{ sizeof(mi) };
		GetMonitorInfo(MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST), &mi);

		SetWindowLongPtr(hwnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(
			hwnd_, HWND_TOP,
			mi.rcMonitor.left, mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW
		);
		isFullscreen_ = true;
		return;
	}

	DWORD style = StyleForMode(mode) | WS_VISIBLE;
	SetWindowLongPtr(hwnd_, GWL_STYLE, style);

	if (isFullscreen_) {
		SetWindowPlacement(hwnd_, &prevPlacement_);        // フルスクリーンから戻す
		isFullscreen_ = false;
	}

	RECT rc{ 0, 0, clientW, clientH };
	AdjustWindowRect(&rc, style, FALSE);
	SetWindowPos(
		hwnd_, nullptr, 0, 0,
		rc.right - rc.left, rc.bottom - rc.top,
		SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED
	);
	// ここでWM_SIZEが飛び、リサイズ処理が自動で動く
}

LRESULT WinAPI::HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

#ifdef USE_IMGUI
	if (ImGuiManager::HandleMessage(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif

	switch (msg) {

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE: {
		if (wparam == SIZE_MINIMIZED) {
			return 0;
		}
		int width = LOWORD(lparam);    // WM_SIZEのときだけlparamは「クライアント領域の幅と高さ」
		int height = HIWORD(lparam);
		if (width > 0 && height > 0) {
			pendingW_ = width;
			pendingH_ = height;
			needResize_ = true;
		}
		return 0;
	}

	case WM_SIZING: {
		// 比率固定モードのときだけ、ドラッグ中のウィンドウ矩形を補正する
		if (config::GetClientWindowMode() != WINDOWED_Scale) {
			break;
		}

		RECT* rc = reinterpret_cast<RECT*>(lparam);
		const int ratioW = config::GetDefaultWidth();
		const int ratioH = config::GetDefaultHeight();

		// タイトルバーや枠が占めるサイズ
		RECT frame{ 0, 0, 0, 0 };
		AdjustWindowRect(&frame, static_cast<DWORD>(GetWindowLongPtr(hwnd, GWL_STYLE)), FALSE);
		const int frameW = frame.right - frame.left;
		const int frameH = frame.bottom - frame.top;

		int clientW = (rc->right - rc->left) - frameW;
		int clientH = (rc->bottom - rc->top) - frameH;

		switch (wparam) {
		case WMSZ_LEFT:
		case WMSZ_RIGHT:                    // 左右を掴んだ → 幅から高さを決める
			clientH = clientW * ratioH / ratioW;
			rc->bottom = rc->top + clientH + frameH;
			break;
		case WMSZ_TOP:
		case WMSZ_BOTTOM:                   // 上下を掴んだ → 高さから幅を決める
			clientW = clientH * ratioW / ratioH;
			rc->right = rc->left + clientW + frameW;
			break;
		default:                            // 角を掴んだ → 幅を基準にする
			clientH = clientW * ratioH / ratioW;
			if (wparam == WMSZ_TOPLEFT || wparam == WMSZ_TOPRIGHT) {
				rc->top = rc->bottom - clientH - frameH;
			} else {
				rc->bottom = rc->top + clientH + frameH;
			}
			break;
		}
		return TRUE;                        // 矩形を書き換えたことをWindowsに伝える
	}

	case WM_ENTERSIZEMOVE:
		sizingMode_ = true;
		return 0;

	case WM_EXITSIZEMOVE:
		sizingMode_ = false;
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);

}

bool WinAPI::RegisterWindowClass() {
	///windowを登録する
	WNDCLASS wc{};
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = className_.c_str();
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	return RegisterClass(&wc) != 0;
};

bool WinAPI::CreateMainWindow(int kClientWidth, int kClientHeight, HINSTANCE hInstance) {

	RECT wrc{ 0,0,kClientWidth ,kClientHeight };
	DWORD currentStyle = StyleForMode(config::GetClientWindowMode());
	AdjustWindowRect(&wrc, currentStyle, FALSE);

	hwnd_ = CreateWindow(
		className_.c_str(),
		className_.c_str(),
		currentStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left, // ウィンドウ横幅
		wrc.bottom - wrc.top, // ウィンドウ縦幅
		nullptr,
		nullptr,
		hInstance,
		this
	);

	return hwnd_ != nullptr;
}

