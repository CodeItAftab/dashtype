#include "dataset_manager.hpp"

#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>

namespace {

bool tryReadLines(const std::vector<std::string>& candidatePaths,
                   std::vector<std::string>& outLines) {
    for (const auto& path : candidatePaths) {
        std::ifstream in(path);
        if (!in) continue;

        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;
            outLines.push_back(line);
        }
        if (!outLines.empty()) return true;
    }
    return false;
}

// Minimal hand-written parser for our own quotes.json format:
// { "quotes": ["...", "..."] }. Not a general JSON parser — just enough
// to read the specific structure this project writes.
bool tryReadQuotesJson(const std::vector<std::string>& candidatePaths,
                       std::vector<std::string>& outQuotes) {
    for (const auto& path : candidatePaths) {
        std::ifstream in(path);
        if (!in) continue;

        std::stringstream buffer;
        buffer << in.rdbuf();
        std::string content = buffer.str();

        std::size_t pos = content.find('[');
        std::size_t end = content.find(']', pos);
        if (pos == std::string::npos || end == std::string::npos) continue;

        std::size_t i = pos + 1;
        while (i < end) {
            std::size_t startQuote = content.find('"', i);
            if (startQuote == std::string::npos || startQuote > end) break;

            std::size_t j = startQuote + 1;
            std::string quote;
            while (j < content.size() && content[j] != '"') {
                if (content[j] == '\\' && j + 1 < content.size()) {
                    quote += content[j + 1];
                    j += 2;
                } else {
                    quote += content[j];
                    ++j;
                }
            }
            outQuotes.push_back(quote);
            i = j + 1;
        }
        if (!outQuotes.empty()) return true;
    }
    return false;
}

struct GeneratorState {
    std::vector<std::string> pool;
    std::string lastWord;
    int wordsUntilSentenceEnd = 8;
    bool capitalizeNext = true;
    bool numbersEnabled = false;
    bool punctuationEnabled = false;
};

std::string pickWord(GeneratorState& state) {
    if (state.pool.empty()) return "";
    std::string word;
    do {
        int idx = std::rand() % static_cast<int>(state.pool.size());
        word = state.pool[idx];
    } while (state.pool.size() > 1 && word == state.lastWord);
    state.lastWord = word;
    return word;
}

}  // namespace

void DatasetManager::loadAll() {
    words_.clear();
    quotes_.clear();

    std::vector<std::string> wordCandidates = {
        "data/words/common-1k.txt",
        "../data/words/common-1k.txt",
    };
    if (!tryReadLines(wordCandidates, words_)) {
        loadFallbackWords();
    }

    std::vector<std::string> quoteCandidates = {
        "data/quotes/quotes.json",
        "../data/quotes/quotes.json",
    };
    if (!tryReadQuotesJson(quoteCandidates, quotes_)) {
        loadFallbackQuotes();
    }
}

void DatasetManager::loadFallbackWords() {
    words_ = {"the", "quick", "brown", "fox",  "jumps",   "over",
              "lazy", "dog",   "keyboard", "practice", "speed", "accuracy"};
}

void DatasetManager::loadFallbackQuotes() {
    quotes_ = {"Practice a little every day and the results will surprise you."};
}

std::vector<std::string> DatasetManager::filterByDifficulty(Difficulty difficulty) const {
    std::vector<std::string> result;
    for (const auto& w : words_) {
        std::size_t len = w.size();
        bool match = false;
        switch (difficulty) {
            case Difficulty::Easy:   match = (len <= 4); break;
            case Difficulty::Medium: match = (len >= 5 && len <= 7); break;
            case Difficulty::Hard:   match = (len >= 8); break;
        }
        if (match) result.push_back(w);
    }
    // If the filtered bucket is too small to feel varied, fall back to the
    // full pool rather than repeating a handful of words constantly.
    if (result.size() < 15) return words_;
    return result;
}

std::function<std::string()> DatasetManager::makeGenerator(const AppConfig& config) const {
    if (config.mode == Mode::Quotes) {
        auto quotesCopy = std::make_shared<std::vector<std::string>>(quotes_);
        auto lastIndex = std::make_shared<int>(-1);
        return [quotesCopy, lastIndex]() -> std::string {
            if (quotesCopy->empty()) return "";
            int idx;
            do {
                idx = std::rand() % static_cast<int>(quotesCopy->size());
            } while (quotesCopy->size() > 1 && idx == *lastIndex);
            *lastIndex = idx;
            return (*quotesCopy)[idx];
        };
    }

    auto state = std::make_shared<GeneratorState>();
    state->pool = filterByDifficulty(config.difficulty);
    state->numbersEnabled = config.numbers;
    state->punctuationEnabled = config.punctuation;
    state->wordsUntilSentenceEnd = 6 + (std::rand() % 6);

    return [state]() -> std::string {
        // Occasionally emit a standalone number instead of a word.
        if (state->numbersEnabled && (std::rand() % 8 == 0)) {
            state->lastWord.clear();
            return std::to_string(1 + std::rand() % 999);
        }

        std::string word = pickWord(*state);
        if (word.empty()) return word;

        if (state->punctuationEnabled) {
            if (state->capitalizeNext) {
                word[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(word[0])));
                state->capitalizeNext = false;
            }

            --state->wordsUntilSentenceEnd;
            if (state->wordsUntilSentenceEnd <= 0) {
                word += '.';
                state->capitalizeNext = true;
                state->wordsUntilSentenceEnd = 6 + (std::rand() % 6);
            } else if (std::rand() % 6 == 0) {
                word += ',';
            }
        }

        return word;
    };
}