#include "cli_parser.hpp"
#include "text_buffer.hpp"
#include "typing_screen.hpp"
#include "results_screen.hpp"
#include "home_screen.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> wordPool = {
    "the", "quick", "brown", "fox", "jumps", "over", "lazy", "dog",
    "keyboard", "practice", "speed", "accuracy", "rhythm", "steady",
    "type", "words", "focus", "flow", "test", "code", "build",
    "project", "system", "design", "simple", "clean", "logic",
};

std::string configSummary(const AppConfig& config) {
    return std::to_string(config.timeSeconds) + "s \u00b7 " +
           toString(config.mode) + " \u00b7 " + toString(config.difficulty);
}

void printHelp() {
    std::cout << "Usage: dashtype <command> [options]\n\n"
              << "Commands:\n"
              << "  start      Start a typing test\n"
              << "  download   Download additional typing material\n"
              << "  update     Update local typing material\n"
              << "  stats      Show personal statistics\n"
              << "  history    Show previous tests\n"
              << "  config     Configure preferences\n"
              << "  version    Show version\n"
              << "  help       Show this help\n\n"
              << "Options for 'start':\n"
              << "  --time <15|30|60>\n"
              << "  --mode <words|quotes>\n"
              << "  --difficulty <easy|medium|hard>\n"
              << "  --punctuation\n"
              << "  --numbers\n";
}

// Runs typing tests using `config`, looping on "New Test" until the user
// picks Exit or quits mid-test.
void runTypingLoop(const AppConfig& config) {
    bool keepGoing = true;
    while (keepGoing) {
        TextBuffer buffer(wordPool);
        std::size_t estimatedChars =
            static_cast<std::size_t>(config.timeSeconds * 13) + 100;
        buffer.ensureAheadOf(0, estimatedChars);

        TypingScreenResult result = runTypingScreen(
            buffer.getText(), config.timeSeconds, configSummary(config));

        if (result.quit || result.backToConfig) {
            keepGoing = false;
            break;
        }

        ResultsAction action = runResultsScreen(
            result.metrics, result.samples, config.timeSeconds);

        keepGoing = (action == ResultsAction::NewTest);
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    AppConfig config = parseArgs(argc, argv);

    switch (config.command) {
        case Command::Start: {
            runTypingLoop(config);
            break;
        }

        case Command::Version:
            std::cout << "dashtype 0.1.0 (development)\n";
            break;

        case Command::Help:
            printHelp();
            break;

        case Command::Home: {
            bool running = true;
            while (running) {
                HomeScreenResult home = runHomeScreen(config);
                config = home.config;

                switch (home.action) {
                    case HomeAction::Start:
                        runTypingLoop(config);
                        break;
                    case HomeAction::Stats:
                        std::cout << "Stats aren't implemented yet.\n";
                        std::cout << "Press Enter to return...";
                        std::cin.get();
                        break;
                    case HomeAction::Help:
                        printHelp();
                        std::cout << "Press Enter to return...";
                        std::cin.get();
                        break;
                    case HomeAction::Quit:
                        running = false;
                        break;
                }
            }
            break;
        }

        case Command::Download:
        case Command::Update:
        case Command::Stats:
        case Command::History:
        case Command::Config:
            std::cout << "This command isn't implemented yet.\n";
            break;

        case Command::Unknown:
        default:
            std::cout << "Unknown command. Run 'dashtype help' for usage.\n";
            break;
    }

    return 0;
}