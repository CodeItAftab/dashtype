#include "dataset_meta.hpp"
#include "app_paths.hpp"
#include <fstream>
#include <ctime>

namespace {
std::string metaFilePath() { return exeDirectory() + "dataset_meta.txt"; }
}

bool datasetNeedsUpdate(int thresholdHours) {
    std::ifstream in(metaFilePath());
    if (!in) return true;

    long long lastUpdate = 0;
    in >> lastUpdate;
    if (!in) return true;

    long long now = static_cast<long long>(std::time(nullptr));
    return ((now - lastUpdate) / 3600) >= thresholdHours;
}

void markDatasetUpdated() {
    std::ofstream out(metaFilePath(), std::ios::trunc);
    if (!out) return;
    out << static_cast<long long>(std::time(nullptr));
}