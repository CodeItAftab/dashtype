#pragma once
#include <string>

struct DownloadResult {
    bool success = false;
    std::string message;
};

// Fetches https://<host><path> and atomically writes the response body to
// destPath (downloads to a temp file, then renames into place, so a failed
// download never corrupts the existing offline dataset).
DownloadResult downloadWordList(const std::string& host, const std::string& path,
                                 const std::string& destPath);