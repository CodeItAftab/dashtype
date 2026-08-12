#pragma once
#include <string>
#include <vector>

/*
    * Manages a continuously-growing stream of words for a typing test.
    * The caller (typing engine / UI) reads from getText(); when the caller's
    * cursro gets close to the end, call ensureAheadOf() to append more words 
    * before the user catches up to the buffer's end.
*/ 

class TextBuffer {
public:
    explicit TextBuffer(std::vector<std::string> wordPool);

    // Returns the full text generated so far, words separated by single spaces.
    const std::string& getText() const;

    // Ensures at least `minAheadChars` characters of untyped text exist
    // beyond  `cursorPos`. Appends more words if needed.
    void ensureAheadOf(std::size_t cursorPos, std::size_t minAheadChars = 60);

private:
    std::vector<std::string> wordPool_;
    std::string text_;
    std::string lastword_;

    void appendWords(int count);
};