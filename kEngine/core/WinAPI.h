#pragma once
#include <windows.h>
#include <cstdint>
#include <iostream>
#include <locale>
#include <codecvt>

#include <vector>
#include <string>
#include "StringManage\ConvertString.h"
#include "Logger.h"
#include "config.h"

class DirectXCore;
class WinAPI
{
public:
	~WinAPI();

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	bool Initialize(const std::string& title, int width, int height, HINSTANCE hInstance = GetModuleHandle(nullptr));
	void Finalize() const;

	//WinAPI* GetWinAPI() { return this; }

	HWND GetHWND() const { return hwnd_; }
	HINSTANCE GetHINSTANCE() const { return hInstance_; }

	bool ProcessMessage();
	void SetClientSize(int width, int height);
	void ApplyWindowMode(WindowMode mode, int clientW, int clientH);

	int GetPendingWidth() { return pendingW_; }
	int GetPendingHeight() { return pendingH_; }
	bool IsResizeNeeded() { return needResize_; }
	bool IsSizingMode() { return sizingMode_; }
	void SetResizeNeeded(bool value) { needResize_ = value; }


private:
	LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	HINSTANCE hInstance_ = {};
	HWND hwnd_ = {};
	std::wstring className_ = L"DefultWindowName_";
	bool isFullscreen_ = false;
	WINDOWPLACEMENT prevPlacement_{};

	int pendingW_ = 0;
	int pendingH_ = 0;
	bool needResize_ = false;
	bool sizingMode_ = false;

	bool RegisterWindowClass();
	bool CreateMainWindow(int kClientWidth, int kClientHeight, HINSTANCE hInstance);
};

/// NOTE: 将来多ウィンドウのために、WinAPIをStateから参照するようにする 