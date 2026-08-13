#pragma once
#include <functional>
#include <string>

// Manages a continuously-growing stream of text for a typing test.
// `nextChunk` supplies the next word/quote on demand — TextBuffer just
// handles spacing and growth, it doesn't care where chunks come from.
class TextBuffer {
public:
    explicit TextBuffer(std::function<std::string()> nextChunk);

    const std::string& getText() const;
    void ensureAheadOf(std::size_t cursorPos, std::size_t minAheadChars = 60);

private:
    std::function<std::string()> nextChunk_;
    std::string text_;

    void appendChunks(int count);
};