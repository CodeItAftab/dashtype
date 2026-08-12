#include "text_buffer.hpp"
#include <cstdlib>

TextBuffer:: TextBuffer(std::vector<std::string> wordPool)
    : wordPool_(std::move(wordPool)) {
        appendWords(20); // seeded with an initial batch so getText() is never empty.
}

const std::string& TextBuffer::getText() const {
    return text_;
}

void TextBuffer::ensureAheadOf(std::size_t cursorPos, std::size_t minAheadChars){
    while(text_.size() < cursorPos + minAheadChars){
        appendWords(10);
    }
}

void TextBuffer::appendWords(int count){
    if(wordPool_.empty()) return;

    if(wordPool_.size() == 1) {
        // Only one word available- nothing to avoid repeating against.
        for(int i = 0; i < count; i++){
            if(!text_.empty()) text_ += ' ';
            text_ += wordPool_[0];
        }
    }

    for(int i = 0; i< count; ++i){
        int index;

        do {
            index = std::rand() % static_cast<int>(wordPool_.size());
        }while(!lastword_.empty() && wordPool_[index] == lastword_);

        if(!text_.empty()){
            text_ += ' ';
        }
        
        text_ += wordPool_[index];
        lastword_ = wordPool_[index];
    }
}