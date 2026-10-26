#pragma once
#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
#include <ostream>

#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

struct Entry {
	std::string time{};
	long long stampMs = 0;
	std::string saveLog{};
};

inline std::vector<Entry> logEntries;

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

	/// <summary>
	/// 出力ウィンドウに文字出力
	/// </summary>
	/// <typeparam name="...Args"></typeparam>
	/// <param name="fmt"></param>
	/// <param name="...args"></param>
	template<typename... Args>
	static void Log(const char* fmt, Args... args) {
		int size = std::snprintf(nullptr, 0, fmt, args...) + 1;
		if (size <= 0) return;

		std::vector<char> buf(size);
		std::snprintf(buf.data(), size, fmt, args...);

		OutputDebugStringA(buf.data());
		OutputDebugStringA("\n");
		SaveLog(buf.data());
	}

	/// ============ ログの保存 ============ ///

	static void SaveLog(const std::string& message);
	static void SaveLog(const std::wstring& message);

	static std::string GetLog();

	/// 画面のログだけ消す（ファイルは残る）
	static void ClearEntries() { logEntries.clear(); }


	/// ====== デバッグログと混ぜて出力用 ====== ///
	static long long NowMs();
	/// クラッシュ時用：最新 count 件を時間順に、crashTime からの時間差付きで書く（最後に CRASH 行）
	static void WriteRecent(std::ostream& out, size_t count, long long crashTimeMs);
	/// 今のログファイルの場所
	static const std::string& GetLogFilePath();

	/// ======== ログをゲーム内で表示 ========= ///
	/// 表示用：保存中のログ（読むだけ）
	static const std::vector<Entry>& GetEntries() { return logEntries; }
};
