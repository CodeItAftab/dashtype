#include "personal_best.hpp"
#include <fstream>
#include <map>

namespace {
const char* kFileName = "dashtype_bests.txt";

std::map<int, double> loadAll() {
    std::map<int, double> bests;
    std::ifstream in(kFileName);
    if (!in) return bests;
    int duration;
    double wpm;
    while (in >> duration >> wpm) {
        bests[duration] = wpm;
    }
    return bests;
}

void saveAll(const std::map<int, double>& bests) {
    std::ofstream out(kFileName, std::ios::trunc);
    for (const auto& [duration, wpm] : bests) {
        out << duration << " " << wpm << "\n";
    }
}
}  // namespace

std::optional<double> loadPersonalBest(int durationSeconds) {
    auto bests = loadAll();
    auto it = bests.find(durationSeconds);
    if (it == bests.end()) return std::nullopt;
    return it->second;
}

bool checkAndUpdatePersonalBest(int durationSeconds, double wpm) {
    auto bests = loadAll();
    auto it = bests.find(durationSeconds);
    bool isNewBest = (it == bests.end()) || (wpm > it->second);
    if (isNewBest) {
        bests[durationSeconds] = wpm;
        saveAll(bests);
    }
    return isNewBest;
}