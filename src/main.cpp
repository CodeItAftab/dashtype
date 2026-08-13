#include "cli_parser.hpp"
#include "text_buffer.hpp"
#include "typing_screen.hpp"
#include "results_screen.hpp"
#include "home_screen.hpp"
#include "dataset_manager.hpp"
#include "database.hpp"
#include "downloader.hpp"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

namespace {

DatasetManager g_dataset;
Database g_db;

const char* kWordListHost = "raw.githubusercontent.com";
const char* kWordListPath = "/first20hours/google-10000-english/master/google-10000-english.txt";
const char* kWordListDest = "data/words/common-1k.txt";

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

void printStats() {
    StatsSummary stats = g_db.overallStats();
    std::cout << "\n--- Stats ---\n";
    std::cout << "Total tests: " << stats.totalTests << "\n";
    std::cout << "Average accuracy: " << stats.avgAccuracy << "%\n";
    for (auto& [duration, best] : stats.bestByDuration) {
        std::cout << "Best WPM (" << duration << "s): ";
        if (best.has_value()) std::cout << *best << "\n";
        else std::cout << "no data yet\n";
    }
}

void printHistory() {
    auto records = g_db.recentHistory(10);
    std::cout << "\n--- Recent Tests ---\n";
    if (records.empty()) {
        std::cout << "No tests recorded yet.\n";
        return;
    }
    for (auto& r : records) {
        std::cout << r.timestamp << "  " << r.durationSeconds << "s  "
                  << r.mode << "/" << r.difficulty << "  "
                  << r.wpm << " WPM  " << r.accuracyPercent << "% acc\n";
    }
}

void runTypingLoop(const AppConfig& config) {
    bool keepGoing = true;
    while (keepGoing) {
        TextBuffer buffer(g_dataset.makeGenerator(config));
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
            result.metrics, result.samples, config, g_db);

        keepGoing = (action == ResultsAction::NewTest);
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    AppConfig config = parseArgs(argc, argv);

    g_dataset.loadAll();
    g_db.open("dashtype.db");

    switch (config.command) {
        case Command::Start:
            runTypingLoop(config);
            break;

        case Command::Version:
            std::cout << "dashtype 0.1.0 (development)\n";
            break;

        case Command::Help:
            printHelp();
            break;

        case Command::Stats:
            printStats();
            break;

        case Command::History:
            printHistory();
            break;

        case Command::Download:
        case Command::Update: {
            std::cout << "Downloading typing material...\n";
            DownloadResult dl = downloadWordList(kWordListHost, kWordListPath, kWordListDest);
            std::cout << dl.message << "\n";
            if (dl.success) g_dataset.loadAll();
            break;
        }

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
                        printStats();
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