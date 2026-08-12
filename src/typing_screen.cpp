#include "typing_screen.hpp"
#include "typing_engine.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

using namespace ftxui;

namespace {

enum class ScreenState { WaitingToStart, Running, ConfirmQuit, Finished };

// Splits text into lines no wider than `width`, breaking on spaces only.
// Returns pairs of (starting index into original text, line content).
std::vector<std::pair<std::size_t, std::string>> wrapText(const std::string& text, int width) {
    std::vector<std::pair<std::size_t, std::string>> lines;
    std::size_t lineStart = 0;
    std::size_t i = 0;
    std::size_t lastSpace = std::string::npos;
    std::size_t lastSpaceLineLen = 0;

    while (i <= text.size()) {
        bool atEnd = (i == text.size());
        std::size_t curLen = i - lineStart;

        if (!atEnd && text[i] == ' ') {
            lastSpace = i;
            lastSpaceLineLen = curLen;
        }

        if (atEnd || curLen >= static_cast<std::size_t>(width)) {
            if (!atEnd && lastSpace != std::string::npos && lastSpace > lineStart) {
                lines.emplace_back(lineStart, text.substr(lineStart, lastSpaceLineLen));
                lineStart = lastSpace + 1;
                i = lineStart;
                lastSpace = std::string::npos;
                continue;
            } else {
                lines.emplace_back(lineStart, text.substr(lineStart, curLen));
                lineStart = i;
                if (atEnd) break;
            }
        }
        ++i;
    }
    if (lines.empty()) lines.emplace_back(0, "");
    return lines;
}

Color timerColor(double remaining, double total) {
    if (total <= 0) return Color::White;
    double pct = remaining / total;
    if (pct > 0.5) return Color::White;
    if (pct > 0.25) return Color::Yellow;
    if (pct > 0.10) return Color::RGB(255, 140, 0);  // orange
    return Color::Red;
}

}  // namespace

TypingScreenResult runTypingScreen(const std::string& targetText, int durationSeconds) {
    TypingEngine engine(targetText);
    TypingScreenResult result;

    ScreenState state = ScreenState::WaitingToStart;
    int confirmSelected = 0;  // 0 = Yes, 1 = No

    std::chrono::steady_clock::time_point startTime;
    std::atomic<bool> tickerRunning{true};
    std::vector<WpmSample> samples;
    int lastSampledSecond = -1;

    auto screen = ScreenInteractive::Fullscreen();

    // Background ticker: wakes the UI loop every 100ms so the timer/state
    // updates even with no keypresses, and takes periodic WPM samples.
    std::thread ticker([&] {
        while (tickerRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (state == ScreenState::Running) {
                double elapsed = std::chrono::duration<double>(
                                      std::chrono::steady_clock::now() - startTime)
                                      .count();
                int wholeSecond = static_cast<int>(elapsed);
                if (wholeSecond != lastSampledSecond) {
                    lastSampledSecond = wholeSecond;
                    int correctSoFar = 0;
                    for (std::size_t j = 0; j < engine.cursorPos() && j < targetText.size(); ++j) {
                        if (engine.stateAt(j) == CharState::Correct) ++correctSoFar;
                    }
                    samples.push_back({elapsed, correctSoFar});
                }
                if (elapsed >= durationSeconds) {
                    state = ScreenState::Finished;
                }
            }
            screen.PostEvent(Event::Custom);
        }
    });

    auto renderer = Renderer([&] {
        double elapsed = (state == ScreenState::WaitingToStart)
            ? 0.0
            : std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
        double remaining = durationSeconds - elapsed;
        if (remaining < 0) remaining = 0;
        int remainingWhole = static_cast<int>(remaining + 0.999);

        Element timerText = text(std::to_string(remainingWhole)) | bold |
                             color(timerColor(remaining, durationSeconds)) | center;

        // Build wrapped lines and find which line the cursor is on.
        auto lines = wrapText(targetText, 60);
        std::size_t cursorLine = 0;
        for (std::size_t li = 0; li < lines.size(); ++li) {
            if (engine.cursorPos() >= lines[li].first) cursorLine = li;
        }

        // Show a window of up to 4 lines, centered around the cursor line.
        int windowSize = 4;
        int startLine = static_cast<int>(cursorLine) - 1;
        if (startLine < 0) startLine = 0;
        int endLine = std::min(static_cast<int>(lines.size()), startLine + windowSize);

        Elements textLines;
        for (int li = startLine; li < endLine; ++li) {
            const auto& [lineStart, content] = lines[li];
            Elements chars;
            for (std::size_t ci = 0; ci < content.size(); ++ci) {
                std::size_t globalIdx = lineStart + ci;
                char c = content[ci];
                Element charEl = text(std::string(1, c));

                if (globalIdx == engine.cursorPos()) {
                    charEl = charEl | inverted;
                } else if (globalIdx < engine.cursorPos()) {
                    CharState st = engine.stateAt(globalIdx);
                    if (st == CharState::Correct) charEl = charEl | color(Color::GrayLight);
                    else if (st == CharState::Incorrect) charEl = charEl | color(Color::Red) | underlined;
                } else {
                    charEl = charEl | dim;
                }
                chars.push_back(charEl);
            }
            // If cursor sits exactly at end-of-line boundary and there are
            // extra chars pending, show them appended (rare edge case: only
            // relevant on the very last line).
            textLines.push_back(hbox(chars));
        }

        Element typingArea = vbox(textLines) | center;

        if (state == ScreenState::WaitingToStart) {
            typingArea = vbox({
                typingArea,
                text(""),
                text("Start typing to begin...") | dim | center,
            });
        }

        Element mainView = vbox({
            timerText,
            text(""),
            typingArea,
        }) | border | center;

        if (state == ScreenState::ConfirmQuit) {
            Element dialog = vbox({
                text("Quit this test?") | bold | center,
                text(""),
                hbox({
                    text(confirmSelected == 0 ? "> Yes  " : "  Yes  ") |
                        (confirmSelected == 0 ? bold : dim),
                    text(confirmSelected == 1 ? "> No" : "  No") |
                        (confirmSelected == 1 ? bold : dim),
                }) | center,
            }) | border | center;
            return dialog;
        }

        return mainView;
    });

    auto component = CatchEvent(renderer, [&](Event event) -> bool {
        if (state == ScreenState::Finished) return false;

        if (state == ScreenState::ConfirmQuit) {
            if (event == Event::ArrowLeft || event == Event::ArrowRight) {
                confirmSelected = 1 - confirmSelected;
                return true;
            }
            if (event == Event::Return) {
                if (confirmSelected == 0) {
                    result.quit = true;
                    state = ScreenState::Finished;
                } else {
                    state = ScreenState::Running;
                }
                return true;
            }
            return true;  // swallow all other input while dialog is open
        }

        if (event == Event::Escape) {
            if (state == ScreenState::Running || state == ScreenState::WaitingToStart) {
                state = ScreenState::ConfirmQuit;
                confirmSelected = 0;
                return true;
            }
        }

        if (event == Event::Backspace) {
            if (state == ScreenState::Running) {
                engine.onBackspace();
            }
            return true;
        }

        if (event.is_character() && !event.character().empty()) {
            char c = event.character()[0];
            if (state == ScreenState::WaitingToStart) {
                state = ScreenState::Running;
                startTime = std::chrono::steady_clock::now();
            }
            if (state == ScreenState::Running) {
                engine.onChar(c);
                if (engine.cursorPos() >= targetText.size()) {
                    state = ScreenState::Finished;
                }
            }
            return true;
        }

        return false;
    });

    // Exit the FTXUI loop once we reach Finished. We poll state via the
    // same ticker-driven redraws.
    auto withExit = CatchEvent(component, [&](Event) {
        if (state == ScreenState::Finished) {
            screen.Exit();
        }
        return false;  // never swallow; just watch for exit condition
    });

    screen.Loop(withExit);

    tickerRunning = false;
    ticker.join();

    double finalDuration = std::chrono::duration<double>(
                                std::chrono::steady_clock::now() - startTime)
                                .count();
    if (finalDuration <= 0 || finalDuration > durationSeconds + 1) {
        finalDuration = durationSeconds;
    }

    result.completed = !result.quit;
    result.metrics = computeResultMetrics(engine, finalDuration, samples);
    return result;
}