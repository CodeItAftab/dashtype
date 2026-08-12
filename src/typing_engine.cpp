#include "typing_engine.hpp"

TypingEngine::TypingEngine(const std::string& targetText)
    :target_(targetText), states_(targetText.size(), CharState::Untyped) {}

void TypingEngine::onChar(char c) {
    if(cursor_ < target_.size()){
        bool correct = (c == target_[cursor_]);
        states_[cursor_] = correct ? CharState::Correct : CharState::Incorrect;
        log_.push_back({c, correct});
        ++cursor_;
    }else{
        // Past the end of target text: this is an "extra" character
        extra_ += c;
        log_.push_back({c, false});
        ++cursor_;
    }
}

void TypingEngine::onBackspace(){
    if(cursor_ == 0) return;

    if(!extra_.empty()){
        extra_.pop_back();
        --cursor_;
        return;
    }

    --cursor_;
    if(cursor_ < states_.size()){
        states_[cursor_] = CharState::Untyped;
    }
}

std::size_t TypingEngine::cursorPos() const {
    return cursor_;
}

CharState TypingEngine::stateAt(std::size_t index) const {
    if(index >= states_.size()) return CharState::Untyped;
    return states_[index];
}

const std::string& TypingEngine::extraChars() const {
    return extra_;
}

const  std::vector<TypedChar>& TypingEngine::keystrokeLog() const {
    return log_;
}

bool TypingEngine::isComplete() const {
    return cursor_ >= target_.size() && extra_.empty();
}