#include "cli_parser.hpp"
#include <string>
#include <vector>

namespace {

Command parseCommand(const std::string& s) {
    if (s == "start") return Command::Start;
    if (s == "download") return Command::Download;
    if (s == "update") return Command::Update;
    if (s == "stats") return Command::Stats;
    if (s == "history") return Command::History;
    if (s == "config") return Command::Config;
    if (s == "version") return Command::Version;
    if (s == "help") return Command::Help;
    return Command::Unknown;
}

Mode parseMode(const std::string& s) {
    if (s == "quotes") return Mode::Quotes;
    return Mode::Words;
}

Difficulty parseDifficulty(const std::string& s) {
    if (s == "easy") return Difficulty::Easy;
    if (s == "hard") return Difficulty::Hard;
    return Difficulty::Medium;
}

}  // namespace

AppConfig parseArgs(int argc, char** argv) {
    AppConfig config;

    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty()) {
        config.command = Command::Home;
        return config;
    }

    config.command = parseCommand(args[0]);

    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];

        if (arg == "--time" && i + 1 < args.size()) {
            config.timeSeconds = std::stoi(args[++i]);
        } else if (arg == "--mode" && i + 1 < args.size()) {
            config.mode = parseMode(args[++i]);
        } else if (arg == "--difficulty" && i + 1 < args.size()) {
            config.difficulty = parseDifficulty(args[++i]);
        } else if (arg == "--punctuation") {
            config.punctuation = true;
        } else if (arg == "--numbers") {
            config.numbers = true;
        }
    }

    return config;
}

std::string toString(Mode mode) {
    switch (mode) {
        case Mode::Words: return "Words";
        case Mode::Quotes: return "Quotes";
    }
    return "Words";
}

std::string toString(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Easy: return "Easy";
        case Difficulty::Medium: return "Medium";
        case Difficulty::Hard: return "Hard";
    }
    return "Medium";
}