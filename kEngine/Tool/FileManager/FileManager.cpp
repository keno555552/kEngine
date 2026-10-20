#include "FileManager.h"
#include <commdlg.h>
#include <filesystem>
#pragma comment(lib, "comdlg32.lib")

std::string FileManager::GetDesktopPath() {
	PWSTR wpath = nullptr;
	std::string result;

	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &wpath))) {
		size_t len = wcslen(wpath);

		std::string buffer(len, '\0');

		size_t converted = 0;
		wcstombs_s(&converted, buffer.data(), buffer.size() + 1, wpath, _TRUNCATE);

		result = buffer;
		CoTaskMemFree(wpath);
	}

	return result;
}

std::string FileManager::EnsureSlash(const std::string& path) {
	if (path.empty()) return path;

	char last = path.back();
	if (last != '\\' && last != '/')
		return path + "\\";

	return path;
}

std::string FileManager::OpenFileDialog(const char* filter, const char* title) {
	char fileName[MAX_PATH] = "";

	OPENFILENAMEA ofn{};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = title;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;   // 作業フォルダを変えさせない

	if (GetOpenFileNameA(&ofn)) return fileName;
	return "";
}

std::string FileManager::ToRelativePath(const std::string& absPath) {
	namespace fs = std::filesystem;
	std::error_code ec;
	fs::path rel = fs::relative(absPath, fs::current_path(), ec);

	/// 作業フォルダの外（".." で始まる）or 失敗 → 絶対パスのまま、区切りだけ統一
	if (ec || rel.empty() || rel.generic_string().rfind("..", 0) == 0) {
		return fs::path(absPath).generic_string();
	}
	return "./" + rel.generic_string();       // generic_string() は区切りが必ず '/'
}

std::string FileManager::ToAssetPath(const std::string& path, bool* ok) {
	namespace fs = std::filesystem;
	if (ok) *ok = true;
	if (path.empty()) return path;

	std::string p = fs::path(path).generic_string();          // 区切りを '/' に統一

	/// 1. もう相対パス → 先頭の "./" だけ外す
	if (fs::path(p).is_relative()) {
		if (p.rfind("./", 0) == 0) p = p.substr(2);
		return p;
	}

	/// 2. 作業フォルダの中 → 相対パスに
	std::error_code ec;
	const std::string rel = fs::relative(p, fs::current_path(), ec).generic_string();
	if (!ec && !rel.empty() && rel.rfind("..", 0) != 0) return rel;

	/// 3. 作業フォルダの外 → "GAME/" か "kEngine/" から後ろを切り出す（repo と output のどちらでも同じ形になる）
	for (const char* root : { "/GAME/", "/kEngine/" }) {
		const size_t pos = p.rfind(root);
		if (pos != std::string::npos) return p.substr(pos + 1);
	}

	/// 4. どれでもない（資産フォルダの外を選んだ）→ 絶対パスのまま
	if (ok) *ok = false;
	return p;
}

std::string FileManager::ToAbsolutePath(const std::string& path) {
	if (path.empty()) return path;
	std::error_code ec;
	return std::filesystem::absolute(path, ec).lexically_normal().generic_string();
}

std::string FileManager::MakeRelativeTo(const std::string& path, const std::string& baseDir) {
	namespace fs = std::filesystem;
	if (path.empty()) return path;
	std::error_code ec;
	const fs::path abs = fs::absolute(path, ec);                 // 作業フォルダ基準の相対でも絶対に直す
	const fs::path rel = fs::relative(abs, fs::absolute(baseDir), ec);
	if (ec || rel.empty()) return abs.generic_string();          // 計算できない → 絶対パス
	return rel.generic_string();
}

std::string FileManager::ResolveFrom(const std::string& path, const std::string& baseDir) {
	namespace fs = std::filesystem;
	if (path.empty() || fs::path(path).is_absolute()) return path;
	return (fs::path(baseDir) / path).lexically_normal().generic_string();   // "a/b/../c" → "a/c"
}

std::string FileManager::GetFileName(const std::string& path) {
	return std::filesystem::path(path).filename().string();
}

std::string FileManager::SaveFileDialog(const char* filter, const char* defaultExt, const std::string& defaultName, const char* title) {
	char fileName[MAX_PATH] = "";
	strncpy_s(fileName, defaultName.c_str(), _TRUNCATE);       // 初期のファイル名

	OPENFILENAMEA ofn{};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetActiveWindow();
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = title;
	ofn.lpstrDefExt = defaultExt;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;   // 上書き確認 + 作業フォルダを変えない

	if (GetSaveFileNameA(&ofn)) return fileName;
	return "";
}

std::string FileManager::ToSafeFileName(const std::string& name) {
	std::string result = name;
	for (char& c : result) {
		if (std::string("\\/:*?\"<>|").find(c) != std::string::npos) c = '_';
	}
	return result.empty() ? "untitled" : result;
}
