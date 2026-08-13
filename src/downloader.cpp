#include "downloader.hpp"
#include <windows.h>
#include <winhttp.h>
#include <fstream>
#include <vector>
#include <cstdio>
#pragma comment(lib, "winhttp.lib")

namespace {

std::wstring toWide(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

bool httpGet(const std::wstring& host, const std::wstring& path, std::string& outBody) {
    HINTERNET hSession = WinHttpOpen(L"dashtype/0.1", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                      WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(),
                                         INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return false; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
                                             nullptr, WINHTTP_NO_REFERER,
                                             WINHTTP_DEFAULT_ACCEPT_TYPES,
                                             WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }

    bool ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                  WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
              WinHttpReceiveResponse(hRequest, nullptr);

    if (ok) {
        DWORD available = 0;
        while (WinHttpQueryDataAvailable(hRequest, &available) && available > 0) {
            std::vector<char> buffer(available);
            DWORD read = 0;
            if (!WinHttpReadData(hRequest, buffer.data(), available, &read)) break;
            outBody.append(buffer.data(), read);
        }
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return ok && !outBody.empty();
}

}  // namespace

DownloadResult downloadWordList(const std::string& host, const std::string& path,
                                 const std::string& destPath) {
    DownloadResult result;
    std::string body;

    if (!httpGet(toWide(host), toWide(path), body) || body.size() < 50) {
        result.success = false;
        result.message = "Download failed or response was empty. Check your internet connection.";
        return result;
    }

    std::string tempPath = destPath + ".tmp";
    {
        std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
        if (!out) {
            result.success = false;
            result.message = "Could not write temporary file.";
            return result;
        }
        out << body;
    }

    std::remove(destPath.c_str());
    if (std::rename(tempPath.c_str(), destPath.c_str()) != 0) {
        result.success = false;
        result.message = "Could not finalize downloaded file.";
        return result;
    }

    result.success = true;
    result.message = "Downloaded " + std::to_string(body.size()) + " bytes successfully.";
    return result;
}