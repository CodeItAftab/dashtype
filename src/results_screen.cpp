#include "results_screen.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>
#include <vector>

using namespace ftxui;

namespace {

struct ChartData {
    std::vector<std::string> rows;
    double maxWpm = 0.0;
    double avgWpm = 0.0;
    double minWpm = 0.0;
    int intervalCount = 0;
};

ChartData buildBarChart(const std::vector<WpmSample>& samples, int height, int barWidth) {
    static const char* kBlocks[] = {" ", "\u2581", "\u2582", "\u2583", "\u2584",
                                     "\u2585", "\u2586", "\u2587", "\u2588"};
    ChartData data;
    data.rows.assign(height, "");
    if (samples.size() < 2) return data;

    std::vector<double> intervalWpm;
    for (std::size_t i = 1; i < samples.size(); ++i) {
        double dt = samples[i].secondsElapsed - samples[i - 1].secondsElapsed;
        int dChars = samples[i].correctCharsSoFar - samples[i - 1].correctCharsSoFar;
        if (dt > 0.0) {
            double wpm = (dChars / 5.0) / (dt / 60.0);
            intervalWpm.push_back(std::max(0.0, wpm));
        }
    }
    if (intervalWpm.empty()) return data;

    data.maxWpm = *std::max_element(intervalWpm.begin(), intervalWpm.end());
    data.minWpm = *std::min_element(intervalWpm.begin(), intervalWpm.end());
    data.avgWpm = std::accumulate(intervalWpm.begin(), intervalWpm.end(), 0.0) /
                  intervalWpm.size();
    data.intervalCount = static_cast<int>(intervalWpm.size());

    double scaleMax = (data.maxWpm <= 0.0) ? 1.0 : data.maxWpm;

    for (double w : intervalWpm) {
        int level = static_cast<int>((w / scaleMax) * (height * 8) + 0.5);
        level = std::clamp(level, 0, height * 8);
        int fullRows = level / 8;
        int remainder = level % 8;

        for (int r = 0; r < height; ++r) {
            int rowFromBottom = height - 1 - r;
            std::string cell = (rowFromBottom < fullRows)   ? kBlocks[8]
                                : (rowFromBottom == fullRows) ? kBlocks[remainder]
                                                               : " ";
            for (int b = 0; b < barWidth; ++b) data.rows[r] += cell;
            data.rows[r] += " ";
        }
    }
    return data;
}

std::string formatFixed(double value, int decimals) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", decimals, value);
    return buf;
}

}  // namespace

ResultsAction runResultsScreen(const ResultMetrics& metrics,
                                const std::vector<WpmSample>& samples,
                                const AppConfig& config,
                                Database& db) {
    auto screen = ScreenInteractive::Fullscreen();
    ResultsAction chosenAction = ResultsAction::NewTest;
    int selected = 0;

    std::optional<double> previousBest = db.personalBest(config.timeSeconds);
    bool isNewBest = !previousBest.has_value() || metrics.wpm > *previousBest;

    TestRecord record;
    record.durationSeconds = config.timeSeconds;
    record.wpm = metrics.wpm;
    record.rawWpm = metrics.rawWpm;
    record.accuracyPercent = metrics.accuracyPercent;
    record.consistencyPercent = metrics.consistencyPercent;
    record.correctChars = metrics.correctChars;
    record.errors = metrics.errors;
    record.mode = toString(config.mode);
    record.difficulty = toString(config.difficulty);
    db.recordTest(record);

    ChartData chart = buildBarChart(samples, 10, 2);

    auto renderer = Renderer([&] {
        Elements content;

        content.push_back(text("RESULTS") | bold | center);
        content.push_back(text(""));

        if (isNewBest) {
            content.push_back(text("\u2605 NEW PERSONAL BEST \u2605") | bold |
                               color(Color::Yellow) | center);
            content.push_back(text(formatFixed(metrics.wpm, 1) + " WPM") |
                               bold | color(Color::Yellow) | center);
            if (previousBest.has_value()) {
                content.push_back(text("Previous best: " +
                                        formatFixed(*previousBest, 1) + " WPM") |
                                   dim | center);
            }
        } else {
            content.push_back(text(formatFixed(metrics.wpm, 1) + " WPM") |
                               bold | color(Color::GreenLight) | center);
            if (previousBest.has_value()) {
                content.push_back(text("Personal best: " +
                                        formatFixed(*previousBest, 1) + " WPM") |
                                   dim | center);
            }
        }

        content.push_back(text(formatFixed(metrics.accuracyPercent, 1) +
                                "% ACCURACY") | color(Color::CyanLight) | center);
        content.push_back(text(""));
        content.push_back(separator());
        content.push_back(text(""));

        content.push_back(hbox({
            text("Raw WPM      ") | dim, text(formatFixed(metrics.rawWpm, 1)),
        }) | center);
        content.push_back(hbox({
            text("Characters   ") | dim,
            text(std::to_string(metrics.correctChars) + " / " +
                 std::to_string(metrics.totalChars)),
        }) | center);
        content.push_back(hbox({
            text("Errors       ") | dim, text(std::to_string(metrics.errors)),
        }) | center);
        content.push_back(hbox({
            text("Consistency  ") | dim,
            text(formatFixed(metrics.consistencyPercent, 1) + "%"),
        }) | center);
        content.push_back(hbox({
            text("Time         ") | dim,
            text(std::to_string(config.timeSeconds) + "s"),
        }) | center);

        content.push_back(text(""));
        content.push_back(separator());
        content.push_back(text(""));

        bool hasGraph = chart.intervalCount > 0;
        if (hasGraph) {
            content.push_back(text("WPM OVER TIME") | dim | center);
            content.push_back(hbox({
                text("Peak " + formatFixed(chart.maxWpm, 0) + "   ") | color(Color::GreenLight),
                text("Avg " + formatFixed(chart.avgWpm, 0) + "   ") | color(Color::CyanLight),
                text("Low " + formatFixed(chart.minWpm, 0)) | color(Color::RGB(255, 140, 0)),
            }) | center);
            content.push_back(text(""));

            Elements chartRows;
            for (std::size_t r = 0; r < chart.rows.size(); ++r) {
                std::string label = "    ";
                if (r == 0) {
                    label = formatFixed(chart.maxWpm, 0);
                } else if (r == chart.rows.size() - 1) {
                    label = "0";
                }
                while (label.size() < 4) label = " " + label;

                chartRows.push_back(hbox({
                    text(label) | dim,
                    text(" \u2502 ") | dim,
                    text(chart.rows[r]) | color(Color::CyanLight),
                }));
            }
            content.push_back(vbox(chartRows) | center);

            std::string axisLine = "     \u2514";
            for (std::size_t i = 0; i < chart.rows[0].size(); ++i) axisLine += "\u2500";
            content.push_back(text(axisLine) | dim | center);
            content.push_back(text("seconds") | dim | center);
        } else {
            content.push_back(text("(not enough data for a graph)") | dim | center);
        }

        content.push_back(text(""));
        content.push_back(separator());

        content.push_back(hbox({
            text(selected == 0 ? "> [N] New Test  " : "  [N] New Test  ") |
                (selected == 0 ? bold : dim),
            text(selected == 1 ? "> [Esc] Exit" : "  [Esc] Exit") |
                (selected == 1 ? bold : dim),
        }) | center);

        Element panel = vbox(content) | border |
                         size(WIDTH, GREATER_THAN, Terminal::Size().dimx - 2);
        return vbox({filler(), panel, filler()});
    });

    auto component = CatchEvent(renderer, [&](Event event) -> bool {
        if (event == Event::ArrowLeft || event == Event::ArrowRight) {
            selected = 1 - selected;
            return true;
        }
        if (event == Event::Character('n') || event == Event::Character('N')) {
            chosenAction = ResultsAction::NewTest;
            screen.Exit();
            return true;
        }
        if (event == Event::Escape) {
            chosenAction = ResultsAction::Exit;
            screen.Exit();
            return true;
        }
        if (event == Event::Return) {
            chosenAction = (selected == 0) ? ResultsAction::NewTest
                                            : ResultsAction::Exit;
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
    return chosenAction;
}