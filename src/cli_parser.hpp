#pragma once
#include <string>

enum class Command {
    Home,
    Start,
    Download,
    Update,
    Stats,
    History,
    Config,
    Version,
    Help,
    Unknown
};

enum class Mode { Words, Quotes };
enum class Difficulty { Easy, Medium, Hard };

struct AppConfig {
    Command command = Command::Home;
    int timeSeconds = 30;
    Mode mode = Mode::Words;
    Difficulty difficulty = Difficulty::Medium;
    bool punctuation = false;
    bool numbers = false;
};

AppConfig parseArgs(int argc, char** argv);

std::string toString(Mode mode);
std::string toString(Difficulty difficulty);