#pragma once
#include <string>
#include <vector>

enum class CharState {Untyped, Correct, Incorrect, Extra};

struct TypedChar {
    char typedChar; // -> what the user actually typed
    bool wasCorrect; // -> was it correct at the time of typing (before any backspace)
};

class TypingEngine {
public:
    explicit TypingEngine(const std::string& targetText);

    // Feed one keystroke (a printable character).
    void onChar(char c);

    //Remove the last typed character (handles both normal and extra chars).
    void onBackspace();

    // Cursor position = index into target text the user is currentlyat.
    // If cursor > target length, the user has typed extra characters.
    std::size_t cursorPos() const;

    // State of a character at a given index in the *target* text.
    // ONly valid for index < target.size().
    CharState stateAt(std::size_t index) const;

    // Extra characters typed beyond the target text length.
    const std::string& extraChars() const;

    // Full log of every keystroke ever made (includingones later corrected).
    // Used later for accuracy/metrics calculations.
    const std::vector<TypedChar>& keystrokeLog() const;

    bool isComplete() const; // cursor reached end of target with no pending extras.

private:
    std::string target_;
    std::vector<CharState> states_;
    std::string extra_;
    std::size_t cursor_ = 0;
    std::vector<TypedChar> log_;
};