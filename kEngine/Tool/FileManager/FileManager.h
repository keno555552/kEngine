#pragma once
#include <string>
#include <windows.h>
#include <shlobj.h>

class FileManager
{
public:
    static std::string GetDesktopPath();
    static std::string EnsureSlash(const std::string& path);
};
