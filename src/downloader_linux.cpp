#include "downloader.hpp"
#include <curl/curl.h>
#include <fstream>
#include <cstdio>

namespace {
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* body = static_cast<std::string*>(userp);
    body->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}
}  // namespace

DownloadResult downloadWordList(const std::string& host, const std::string& path,
                                 const std::string& destPath) {
    DownloadResult result;
    std::string url = "https://" + host + path;
    std::string body;

    CURL* curl = curl_easy_init();
    if (!curl) {
        result.message = "Could not initialize curl.";
        return result;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "dashtype/0.1");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || body.size() < 50) {
        result.success = false;
        result.message = "Download failed or response was empty. Check your internet connection.";
        return result;
    }

    std::string tempPath = destPath + ".tmp";
    {
        std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
        if (!out) {
            result.message = "Could not write temporary file.";
            return result;
        }
        out << body;
    }

    std::remove(destPath.c_str());
    if (std::rename(tempPath.c_str(), destPath.c_str()) != 0) {
        result.message = "Could not finalize downloaded file.";
        return result;
    }

    result.success = true;
    result.message = "Downloaded " + std::to_string(body.size()) + " bytes successfully.";
    return result;
}