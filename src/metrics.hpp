#pragma once
#include  "typing_engine.hpp"
#include <vector>


// A periodic snapshot of progress, taken roughlyonce per second during
// a live test. Used for the WPM graph and consistency calculation.
struct WpmSample {
    double secondsElapsed = 0.0;
    int correctCharsSoFar = 0;
};

struct ResultMetrics {
    double wpm = 0.0;
    double rawWpm = 0.0;
    double accuracyPercent = 0.0;
    double consistencyPercent = 0.0;
    int correctChars = 0;
    int incorrectChars = 0;
    int totalChars = 0;
    int errors = 0;
    double durationSeconds = 0.0;
};


// ---Design decision ------------------------------------
// * WPM is net WPM: correct characters(final state) / 5 / minutes.
// * Raw WPM counts every keystroke ever made (including corrected ones) / 5 / minutes.
// * Accuracy = (total Keystrokes - keystrokes that were wrong when typed) / total keystrokes * 100. A mistake that's later back-spaced and fixed still counts against accuracy
// * Consistency = 100 * (1 - stddev(sampleWpm) / mean(sampleWpm)), clampled to [0, 100]. Requires periodic WpmSamples collected during the test.
// -------------------------------------------------------

// Computes final metrics from a completed (or in-progress) TypingEngine,
// the elapsed test duration, and optional periodic samples for consistency.
ResultMetrics computeResultMetrics(const TypingEngine& engine, double durationSeconds, const std::vector<WpmSample>& samples = {});
