#pragma once
#include <string>
#include <windows.h>
#include <shlobj.h>

class FileManager
{
public:
    static std::string GetDesktopPath();
    static std::string EnsureSlash(const std::string& path);
    static std::string OpenFileDialog(const char* filter, const char* title = "Open");
	static std::string ToRelativePath(const std::string& absPath);
	static std::string ToAssetPath(const std::string& path, bool* ok = nullptr);
    static std::string ToAbsolutePath(const std::string& path);
    static std::string MakeRelativeTo(const std::string& path, const std::string& baseDir);
    static std::string ResolveFrom(const std::string& path, const std::string& baseDir);
	static std::string GetFileName(const std::string& path);
	static std::string SaveFileDialog(
        const char* filter,
        const char* defaultExt,
        const std::string& defaultName,
        const char* title = "Save");
    static std::string ToSafeFileName(const std::string& name);
};
