#pragma once
#include "metrics.hpp"
#include "cli_parser.hpp"
#include "database.hpp"
#include <vector>

enum class ResultsAction { NewTest, Exit };

ResultsAction runResultsScreen(const ResultMetrics& metrics,
                                const std::vector<WpmSample>& samples,
                                const AppConfig& config,
                                Database& db);