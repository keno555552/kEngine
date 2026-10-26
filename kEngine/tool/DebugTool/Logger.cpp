#include "Logger.h"
#include <format>
#include <Windows.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <algorithm>

namespace {
	constexpr size_t kMaxLogEntries = 2000;   // 保存する最大件数

	/// 指定時刻を "10:53:09.882" の形で
	std::string TimeText(std::chrono::system_clock::time_point t) {
		const auto local = std::chrono::zoned_time{
			std::chrono::current_zone(), std::chrono::floor<std::chrono::milliseconds>(t) };
		return std::format("{:%H:%M:%S}", local);
	}

	const std::string& LogFilePath() {
		static const std::string path = [] {
			const auto now = std::chrono::zoned_time{ std::chrono::current_zone(), std::chrono::system_clock::now() };
			return std::format("./logs/kEngine_{:%Y%m%d_%H%M%S}.log",
				std::chrono::floor<std::chrono::seconds>(now.get_local_time()));
			}();
		return path;
	}

	std::ofstream& LogFile() {
		static std::ofstream file = [] {
			std::error_code ec;
			std::filesystem::create_directories("./logs", ec);
			return std::ofstream(LogFilePath());
			}();
		return file;
	}

	/// wstring → string（UTF-8）
	std::string ToUtf8(const std::wstring& w) {
		if (w.empty()) return {};
		const int size = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
		std::string out(size, '\0');
		WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), out.data(), size, nullptr, nullptr);
		return out;
	}
}

void Logger::Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
	SaveLog(message);
}

void Logger::Log(const std::wstring& message) {
	OutputDebugStringW(message.c_str());
	SaveLog(message);
}

void Logger::SaveLog(const std::string& message) {

	/// まずはテータを作る
	const auto now = std::chrono::system_clock::now();
	Entry entry{
		.time = TimeText(now),
		.stampMs = NowMs(),
		.saveLog = message,
	};

	/// ファイルに 1 行（落ちても残るように毎回 flush）
	std::ofstream& file = LogFile();
	if (file.is_open()) {
		file << "[" << entry.time << "] " << entry.saveLog;
		if (entry.saveLog.empty() || entry.saveLog.back() != '\n') file << '\n';
		file.flush();
	}

	/// データ追加
	logEntries.push_back(entry);

	/// 一番古いデータを削除
	if (logEntries.size() > kMaxLogEntries) {
		logEntries.erase(logEntries.begin());
	}
}

void Logger::SaveLog(const std::wstring& message) {
	SaveLog(ToUtf8(message));
}

std::string Logger::GetLog() {
	std::string logText;
	for (const auto& entry : logEntries) {
		logText += "[" + entry.time + "] " + entry.saveLog + "\n";
	}
	return logText;
}

long long Logger::NowMs() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void Logger::WriteRecent(std::ostream& out, size_t count, long long crashTimeMs) {
	/// 最新 count 件をコピー（クラッシュ中なので元は触らない）
	const size_t begin = logEntries.size() > count ? logEntries.size() - count : 0;
	std::vector<Entry> recent(logEntries.begin() + begin, logEntries.end());

	/// 時間順に並べる（同じ時刻なら元の順番を保つ）
	std::stable_sort(recent.begin(), recent.end(),
		[](const Entry& a, const Entry& b) { return a.stampMs < b.stampMs; });

	for (const Entry& e : recent) {
		const double diff = std::chrono::duration<double>(e.stampMs - crashTimeMs).count();   // クラッシュ前はマイナス
		out << std::format("[{}] ({:+.3f}s) {}", e.time, diff, e.saveLog);
		if (e.saveLog.empty() || e.saveLog.back() != '\n') out << '\n';
	}
	out << std::format("[{}] (+0.000s) ===== CRASH =====\n", TimeText(std::chrono::system_clock::now()));
}

const std::string& Logger::GetLogFilePath() { return LogFilePath(); }



