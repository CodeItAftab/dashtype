#pragma once
#include "metrics.hpp"
#include <vector>

enum class ResultsAction { NewTest, Exit };

// Shows the results screen and blocks until the user picks an action.
ResultsAction runResultsScreen(const ResultMetrics& metrics,
                                const std::vector<WpmSample>& samples,
                                int durationSeconds);