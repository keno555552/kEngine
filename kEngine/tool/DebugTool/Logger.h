#pragma once
#include <cstdint>
#include <iostream>
#include <vector>

#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

class Logger
{
	public:
	/// <summary>
	/// 出力ウィンドウに文字出力
	/// </summary>
	/// <param name="message"></param>
	static void Log(const std::string& message);

	/// <summary>
	/// 出力ウィンドウに文字出力
	/// </summary>
	/// <param name="message"></param>
	static void Log(const std::wstring& message);
};

