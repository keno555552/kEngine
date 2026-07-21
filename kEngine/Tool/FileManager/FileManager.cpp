#include "FileManager.h"

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
