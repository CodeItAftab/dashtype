#pragma once
#include "metrics.hpp"
#include <string>
#include <vector>

struct TypingScreenResult {
    bool completed = false;    // timer ran out naturally
    bool quit = false;         // user confirmed quit mid-test
    bool backToConfig = false; // user pressed Esc before typing started
    ResultMetrics metrics;
    std::vector<WpmSample> samples;
};

// Runs the interactive typing test screen. `configSummary` is a short
// string (e.g. "30s · Words · Medium") shown in the footer before the
// test starts.
TypingScreenResult runTypingScreen(const std::string& targetText, int durationSeconds,
                                    const std::string& configSummary);