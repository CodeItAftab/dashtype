#pragma once
#include "cli_parser.hpp"

AppConfig loadConfig();
void saveConfig(const AppConfig& config);