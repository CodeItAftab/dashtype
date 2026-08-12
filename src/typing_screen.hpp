#pragma once
#include "metrics.hpp"
#include "cli_parser.hpp"
#include <string>

struct TypingScreenResult {
    bool completed = false;   // true if the timer ran out naturally
    bool quit = false;        // true if the user quit via Esc confirmation
    ResultMetrics metrics;
};

// Runs the interactive typing test screen. Blocks until the test ends
// (timer expires) or the user quits. `targetText` should already contain
// enough words for the configured duration.
TypingScreenResult runTypingScreen(const std::string& targetText, int durationSeconds);