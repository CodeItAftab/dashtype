#include "home_screen.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include <algorithm>
#include <array>
#include <string>
#include <vector>

using namespace ftxui;

namespace {

enum Row { RowTime = 0, RowMode, RowDifficulty, RowPunctuation, RowNumbers, RowStart, RowCount };

const std::array<int, 3> kTimeOptions = {15, 30, 60};
const std::array<Mode, 2> kModeOptions = {Mode::Words, Mode::Quotes};
const std::array<Difficulty, 3> kDifficultyOptions = {Difficulty::Easy, Difficulty::Medium, Difficulty::Hard};

int indexOfTime(int time) {
    for (std::size_t i = 0; i < kTimeOptions.size(); ++i)
        if (kTimeOptions[i] == time) return static_cast<int>(i);
    return 1;
}

int indexOfMode(Mode mode) {
    for (std::size_t i = 0; i < kModeOptions.size(); ++i)
        if (kModeOptions[i] == mode) return static_cast<int>(i);
    return 0;
}

int indexOfDifficulty(Difficulty d) {
    for (std::size_t i = 0; i < kDifficultyOptions.size(); ++i)
        if (kDifficultyOptions[i] == d) return static_cast<int>(i);
    return 1;
}

}  // namespace

HomeScreenResult runHomeScreen(AppConfig initialConfig) {
    auto screen = ScreenInteractive::Fullscreen();

    AppConfig config = initialConfig;
    int selectedRow = RowStart;

    int timeIdx = indexOfTime(config.timeSeconds);
    int modeIdx = indexOfMode(config.mode);
    int difficultyIdx = indexOfDifficulty(config.difficulty);

    HomeScreenResult result;
    result.config = config;

    auto syncConfig = [&] {
        config.timeSeconds = kTimeOptions[timeIdx];
        config.mode = kModeOptions[modeIdx];
        config.difficulty = kDifficultyOptions[difficultyIdx];
    };

    auto rowLabel = [&](int row, const std::string& label, const std::string& value) {
        bool sel = (selectedRow == row);
        Element marker = text(sel ? "> " : "  ");
        Element labelEl = text(label) | size(WIDTH, EQUAL, 14);
        Element valueEl = text("[ " + value + " ]") | (sel ? bold : dim);
        Element rowEl = hbox({marker, labelEl, valueEl});
        return sel ? rowEl | color(Color::CyanLight) : rowEl;
    };

    auto renderer = Renderer([&] {
        Elements content;

        content.push_back(text("D A S H T Y P E") | bold | color(Color::CyanLight) | center);
        content.push_back(text("\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500") |
                           dim | color(Color::CyanLight) | center);
        content.push_back(text("offline-first typing practice") | dim | center);
        content.push_back(text(""));
        content.push_back(separator());
        content.push_back(text(""));

        content.push_back(rowLabel(RowTime, "Time",
                                    std::to_string(kTimeOptions[timeIdx]) + "s") | center);
        content.push_back(rowLabel(RowMode, "Mode",
                                    toString(kModeOptions[modeIdx])) | center);
        content.push_back(rowLabel(RowDifficulty, "Difficulty",
                                    toString(kDifficultyOptions[difficultyIdx])) | center);
        content.push_back(rowLabel(RowPunctuation, "Punctuation",
                                    config.punctuation ? "On" : "Off") | center);
        content.push_back(rowLabel(RowNumbers, "Numbers",
                                    config.numbers ? "On" : "Off") | center);

        content.push_back(text(""));

        bool startSelected = (selectedRow == RowStart);
        content.push_back(text(startSelected ? "> [ START ] <" : "  [ START ]  ") |
                           bold | (startSelected ? color(Color::GreenLight) : dim) | center);

        content.push_back(text(""));
        content.push_back(separator());
        content.push_back(hbox({
            text("\u2191\u2193") | bold, text(" Navigate   "),
            text("\u2190\u2192") | bold, text(" Change   "),
            text("Enter") | bold, text(" Select   "),
            text("T") | bold, text(" Stats   "),
            text("H") | bold, text(" Help   "),
            text("Esc") | bold, text(" Quit"),
        }) | dim | center);

        int boxWidth = std::min(60, Terminal::Size().dimx - 2);
        Element panel = vbox(content) | border | size(WIDTH, GREATER_THAN, boxWidth);
        return vbox({filler(), panel, filler()});
    });

    auto component = CatchEvent(renderer, [&](Event event) -> bool {
        if (event == Event::ArrowUp) {
            selectedRow = (selectedRow - 1 + RowCount) % RowCount;
            return true;
        }
        if (event == Event::ArrowDown) {
            selectedRow = (selectedRow + 1) % RowCount;
            return true;
        }
        if (event == Event::ArrowLeft || event == Event::ArrowRight) {
            int dir = (event == Event::ArrowRight) ? 1 : -1;
            switch (selectedRow) {
                case RowTime:
                    timeIdx = (timeIdx + dir + static_cast<int>(kTimeOptions.size())) %
                              static_cast<int>(kTimeOptions.size());
                    break;
                case RowMode:
                    modeIdx = (modeIdx + dir + static_cast<int>(kModeOptions.size())) %
                              static_cast<int>(kModeOptions.size());
                    break;
                case RowDifficulty:
                    difficultyIdx = (difficultyIdx + dir + static_cast<int>(kDifficultyOptions.size())) %
                                     static_cast<int>(kDifficultyOptions.size());
                    break;
                case RowPunctuation:
                    config.punctuation = !config.punctuation;
                    break;
                case RowNumbers:
                    config.numbers = !config.numbers;
                    break;
                default:
                    break;
            }
            return true;
        }
        if (event == Event::Character('t') || event == Event::Character('T')) {
            syncConfig();
            result.action = HomeAction::Stats;
            result.config = config;
            screen.Exit();
            return true;
        }
        if (event == Event::Character('h') || event == Event::Character('H')) {
            syncConfig();
            result.action = HomeAction::Help;
            result.config = config;
            screen.Exit();
            return true;
        }
        if (event == Event::Return) {
            if (selectedRow == RowStart) {
                syncConfig();
                result.action = HomeAction::Start;
                result.config = config;
                screen.Exit();
            }
            return true;
        }
        if (event == Event::Escape) {
            syncConfig();
            result.action = HomeAction::Quit;
            result.config = config;
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
    return result;
}