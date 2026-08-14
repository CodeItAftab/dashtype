#include "../src/metrics.hpp"
#include "../src/typing_engine.hpp"
#include <cassert>
#include <cmath>
#include <cstdio>

bool approxEqual(double a, double b, double tolerance = 0.05) {
    return std::fabs(a - b) < tolerance;
}

void test_perfect_typing() {
    TypingEngine engine("hello");
    for (char c : std::string("hello")) engine.onChar(c);

    ResultMetrics m = computeResultMetrics(engine, 6.0);  // 5 chars / 5 / (6/60) = 10 WPM
    assert(approxEqual(m.wpm, 10.0));
    assert(approxEqual(m.rawWpm, 10.0));
    assert(approxEqual(m.accuracyPercent, 100.0));
    assert(m.errors == 0);
    printf("test_perfect_typing passed\n");
}

void test_one_uncorrected_error() {
    TypingEngine engine("cat");
    engine.onChar('c');
    engine.onChar('x');  // wrong, left uncorrected
    engine.onChar('t');

    ResultMetrics m = computeResultMetrics(engine, 3.0);
    // 2 correct chars out of cursor 3 -> wpm based on correct=2
    assert(m.correctChars == 2);
    assert(m.incorrectChars == 1);
    assert(m.errors == 1);
    assert(approxEqual(m.accuracyPercent, 66.6667, 0.1));
    printf("test_one_uncorrected_error passed\n");
}

void test_corrected_error_still_counts_against_accuracy() {
    TypingEngine engine("cat");
    engine.onChar('c');
    engine.onChar('x');  // wrong
    engine.onBackspace();
    engine.onChar('a');  // fixed
    engine.onChar('t');

    ResultMetrics m = computeResultMetrics(engine, 4.0);
    assert(m.correctChars == 3);   // final visible state: all correct
    assert(m.errors == 1);         // but the 'x' keystroke still counts
    assert(approxEqual(m.accuracyPercent, 75.0));  // 3 correct / 4 total keystrokes
    printf("test_corrected_error_still_counts_against_accuracy passed\n");
}

int main() {
    test_perfect_typing();
    test_one_uncorrected_error();
    test_corrected_error_still_counts_against_accuracy();
    printf("All metrics tests passed.\n");
    return 0;
}