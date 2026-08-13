#pragma once
#include "cli_parser.hpp"
#include <functional>
#include <string>
#include <vector>

class DatasetManager {
public:
    // Attempts to load words/quotes from data/ files (tries a couple of
    // relative paths since the working directory can vary). Falls back to
    // small built-in lists if nothing is found, so the app is never left
    // with zero material — matches the offline-first requirement.
    void loadAll();

    // Returns a chunk-generator function suitable for TextBuffer, already
    // configured for the given mode/difficulty/punctuation/numbers settings.
    std::function<std::string()> makeGenerator(const AppConfig& config) const;

private:
    std::vector<std::string> words_;
    std::vector<std::string> quotes_;

    void loadFallbackWords();
    void loadFallbackQuotes();
    std::vector<std::string> filterByDifficulty(Difficulty difficulty) const;
};