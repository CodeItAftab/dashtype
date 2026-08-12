#include "cli_parser.hpp"
#include "text_buffer.hpp"
#include "typing_screen.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>

int main(int argc, char** argv) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    AppConfig config = parseArgs(argc, argv);

    std::vector<std::string> wordPool = {
        "the", "quick", "brown", "fox", "jumps", "over", "lazy", "dog",
        "keyboard", "practice", "speed", "accuracy", "rhythm", "steady",
        "type", "words", "focus", "flow", "test", "code", "build",
        "project", "system", "design", "simple", "clean", "logic",
    };

    TextBuffer buffer(wordPool);
    // Generous pre-generation: assume up to ~150 WPM = 12.5 chars/sec.
    std::size_t estimatedChars = static_cast<std::size_t>(config.timeSeconds * 13) + 100;
    buffer.ensureAheadOf(0, estimatedChars);

    TypingScreenResult result = runTypingScreen(buffer.getText(), config.timeSeconds);

    if (result.quit) {
        std::cout << "Test aborted.\n";
        return 0;
    }

    const auto& m = result.metrics;
    std::cout << "\n--- Results ---\n";
    std::cout << "WPM: " << m.wpm << "\n";
    std::cout << "Raw WPM: " << m.rawWpm << "\n";
    std::cout << "Accuracy: " << m.accuracyPercent << "%\n";
    std::cout << "Consistency: " << m.consistencyPercent << "%\n";
    std::cout << "Correct chars: " << m.correctChars << "\n";
    std::cout << "Errors: " << m.errors << "\n";
    std::cout << "Duration: " << m.durationSeconds << "s\n";

    return 0;
}