#pragma once
#include "cli_parser.hpp"

enum class HomeAction { Start, Stats, Help, Quit };

struct HomeScreenResult {
    HomeAction action = HomeAction::Quit;
    AppConfig config;
};

// Shows the interactive home/settings screen (Screen 1). Lets the user
// adjust test settings with the keyboard before starting a test, or jump
// to stats/help. Returns the chosen action and the (possibly modified)
// config to use if action == HomeAction::Start.
HomeScreenResult runHomeScreen(AppConfig initialConfig);