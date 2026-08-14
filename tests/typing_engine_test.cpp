#include "../src/typing_engine.hpp"
#include <cassert>
#include <cstdio>

void test_correct_typing() {
    TypingEngine engine("hi");
    engine.onChar('h');
    engine.onChar('i');
    assert(engine.stateAt(0) == CharState::Correct);
    assert(engine.stateAt(1) == CharState::Correct);
    assert(engine.isComplete());
    printf("test_correct_typing passed\n");
}

void test_incorrect_then_backspace() {
    TypingEngine engine("hi");
    engine.onChar('x');
    assert(engine.stateAt(0) == CharState::Incorrect);
    engine.onBackspace();
    assert(engine.stateAt(0) == CharState::Untyped);
    assert(engine.cursorPos() == 0);
    printf("test_incorrect_then_backspace passed\n");
}

void test_extra_characters() {
    TypingEngine engine("hi");
    engine.onChar('h');
    engine.onChar('i');
    engine.onChar('!');  // extra
    assert(engine.extraChars() == "!");
    assert(!engine.isComplete());
    engine.onBackspace();
    assert(engine.extraChars().empty());
    assert(engine.isComplete());
    printf("test_extra_characters passed\n");
}

void test_keystroke_log_persists_through_correction() {
    TypingEngine engine("a");
    engine.onChar('x');
    engine.onBackspace();
    engine.onChar('a');
    assert(engine.keystrokeLog().size() == 2);  // both keystrokes logged
    assert(engine.keystrokeLog()[0].wasCorrect == false);
    assert(engine.keystrokeLog()[1].wasCorrect == true);
    printf("test_keystroke_log_persists_through_correction passed\n");
}

int main() {
    test_correct_typing();
    test_incorrect_then_backspace();
    test_extra_characters();
    test_keystroke_log_persists_through_correction();
    printf("All typing_engine tests passed.\n");
    return 0;
}