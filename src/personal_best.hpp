#pragma once
#include <optional>

// Placeholder local-storage for personal records, keyed by test duration.
// Stores to a plain text file next to the executable. This will be
// replaced by the SQLite-backed database module described in the spec
// once that layer is built — the results screen only depends on these
// two functions, so swapping the implementation later won't touch the UI.

std::optional<double> loadPersonalBest(int durationSeconds);

// Returns true if this WPM is a new personal best (and saves it if so).
bool checkAndUpdatePersonalBest(int durationSeconds, double wpm);