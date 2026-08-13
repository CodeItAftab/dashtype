#include "text_buffer.hpp"

TextBuffer::TextBuffer(std::function<std::string()> nextChunk)
    : nextChunk_(std::move(nextChunk)) {
    appendChunks(20);
}

const std::string& TextBuffer::getText() const {
    return text_;
}

void TextBuffer::ensureAheadOf(std::size_t cursorPos, std::size_t minAheadChars) {
    while (text_.size() < cursorPos + minAheadChars) {
        appendChunks(10);
    }
}

void TextBuffer::appendChunks(int count) {
    if (!nextChunk_) return;
    for (int i = 0; i < count; ++i) {
        std::string chunk = nextChunk_();
        if (chunk.empty()) continue;
        if (!text_.empty()) text_ += ' ';
        text_ += chunk;
    }
}