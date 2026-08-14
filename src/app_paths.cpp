#include "app_paths.hpp"
#include <windows.h>
#include <string>

std::string exeDirectory() {
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        return "";  // fall back to relative paths if this ever fails
    }

    std::string path(buffer, len);
    std::size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash == std::string::npos) return "";

    return path.substr(0, lastSlash + 1);
}