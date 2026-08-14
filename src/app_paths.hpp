#pragma once
#include <string>

// Returns the absolute directory containing the running executable,
// with a trailing slash. Used so dashtype.exe behaves like a real
// installed tool: all its data lives next to it regardless of what
// folder the user happens to be standing in when they run it.
std::string exeDirectory();