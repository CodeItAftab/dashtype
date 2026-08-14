#include "config_store.hpp"
#include "app_paths.hpp"
#include <fstream>
#include <map>

namespace {
std::string configFilePath() { return exeDirectory() + "dashtype_settings.txt"; }

std::string toStr(Mode m) { return m == Mode::Quotes ? "quotes" : "words"; }
std::string toStr(Difficulty d) {
    switch (d) {
        case Difficulty::Easy: return "easy";
        case Difficulty::Hard: return "hard";
        default: return "medium";
    }
}
Mode modeFromStr(const std::string& s) { return s == "quotes" ? Mode::Quotes : Mode::Words; }
Difficulty difficultyFromStr(const std::string& s) {
    if (s == "easy") return Difficulty::Easy;
    if (s == "hard") return Difficulty::Hard;
    return Difficulty::Medium;
}
}  // namespace

AppConfig loadConfig() {
    AppConfig config;
    std::ifstream in(configFilePath());
    if (!in) return config;

    std::map<std::string, std::string> values;
    std::string key, value;
    while (in >> key >> value) values[key] = value;

    if (values.count("time")) config.timeSeconds = std::stoi(values["time"]);
    if (values.count("mode")) config.mode = modeFromStr(values["mode"]);
    if (values.count("difficulty")) config.difficulty = difficultyFromStr(values["difficulty"]);
    if (values.count("punctuation")) config.punctuation = (values["punctuation"] == "1");
    if (values.count("numbers")) config.numbers = (values["numbers"] == "1");
    return config;
}

void saveConfig(const AppConfig& config) {
    std::ofstream out(configFilePath(), std::ios::trunc);
    if (!out) return;
    out << "time " << config.timeSeconds << "\n";
    out << "mode " << toStr(config.mode) << "\n";
    out << "difficulty " << toStr(config.difficulty) << "\n";
    out << "punctuation " << (config.punctuation ? "1" : "0") << "\n";
    out << "numbers " << (config.numbers ? "1" : "0") << "\n";
}