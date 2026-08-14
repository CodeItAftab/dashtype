#include "app_paths.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <climits>
#include <unistd.h>
#endif

#include <string>

std::string exeDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH) return "";
    std::string path(buffer, len);
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len == -1) return "";
    std::string path(buffer, static_cast<std::size_t>(len));
#endif

    std::size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash == std::string::npos) return "";
    return path.substr(0, lastSlash + 1);
}