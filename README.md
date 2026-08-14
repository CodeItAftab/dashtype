# dashtype

A fast, offline-first, terminal-based typing practice tool for Windows and Linux — inspired by the typing feel of Monkeytype, built from scratch in C++.

```
   ▓▓▓  D A S H T Y P E  ▓▓▓
   offline-first typing practice
```

## Features

- 🖥️ **Real terminal UI** — built with [FTXUI](https://github.com/ArthurSonzogni/FTXUI), not a plain text scroller
- ⌨️ **Live character-by-character feedback** — correct, incorrect, and extra characters are tracked and styled individually, with full backspace support
- ⏱️ **Configurable timed tests** — 15s / 30s / 60s, with a color-shifting countdown timer
- 📝 **Words and Quotes modes**, with adjustable Difficulty, Punctuation, and Numbers
- 📊 **Detailed results screen** — WPM, raw WPM, accuracy, consistency, error count, and a terminal bar-chart graph of WPM over time
- 🏆 **Personal bests**, tracked per test duration
- 💾 **Local history and stats**, stored in SQLite — no account, no cloud, nothing leaves your machine
- 📡 **Optional dataset downloads** — fetch a larger word list when you have internet; the app works fully offline otherwise
- ⚙️ **Persistent settings** — your preferred time/mode/difficulty are remembered between runs
- 🐧🪟 **Cross-platform** — builds and runs natively on both Windows and Linux from the same source

## Quick install

**Windows (PowerShell):**
```powershell
irm https://raw.githubusercontent.com/CodeItAftab/dashtype/main/install.ps1 | iex
```

**Linux / WSL (bash):**
```bash
curl -fsSL https://raw.githubusercontent.com/CodeItAftab/dashtype/main/install.sh | bash
```

Both scripts download the latest packaged release, install it locally, and add it to your PATH so you can run `dashtype` from anywhere. See [Building from source](#building-from-source) if you'd rather build it yourself.

## Usage

```
dashtype                   Open the interactive home/settings screen
dashtype start              Start a test with your saved/default settings
dashtype start --time 30 --mode words --difficulty hard --punctuation --numbers
dashtype stats              Show your personal stats
dashtype history              Show your recent test results
dashtype download           Force re-download typing material
dashtype update             Refresh typing material only if it's stale
dashtype config             Change and save your default settings
dashtype version            Show version
dashtype help               Show usage
```

### Keyboard controls

| Screen | Key | Action |
|---|---|---|
| Home | ↑ / ↓ | Navigate settings |
| Home | ← / → | Change selected setting |
| Home | Enter | Start test / confirm |
| Home | T | View stats |
| Home | H | Help |
| Home | Esc | Quit |
| Typing | (any key) | Type |
| Typing | Backspace | Fix a mistake |
| Typing | Esc | Quit (confirms if mid-test) |
| Results | N | Start a new test |
| Results | Esc | Return to menu |

## Building from source

### Requirements
- CMake ≥ 3.16
- A C++17 compiler (MSVC on Windows, GCC/Clang on Linux)
- Git (for fetching dependencies)
- Linux only: `libcurl4-openssl-dev`
- Internet access for the *first* build only (fetches FTXUI + SQLite via CMake FetchContent)

### Windows
```powershell
git clone https://github.com/CodeItAftab/dashtype.git
cd dashtype
mkdir build
cd build
cmake ..
cmake --build . --config Release
.\Release\dashtype.exe
```

### Linux
```bash
sudo apt install build-essential cmake git libcurl4-openssl-dev
git clone https://github.com/CodeItAftab/dashtype.git
cd dashtype
mkdir build && cd build
cmake ..
cmake --build .
./dashtype
```

### Running tests
```bash
ctest --output-on-failure
```

### Packaging a distributable build
```bash
cpack -C Release      # Windows -> .zip
cpack                 # Linux   -> .tar.gz
```

## Project structure

```
dashtype/
├── CMakeLists.txt
├── install.ps1                Windows one-line installer
├── install.sh                 Linux one-line installer
├── src/
│   ├── main.cpp
│   ├── cli_parser.*            CLI argument parsing
│   ├── text_buffer.*           Continuously-growing typing text stream
│   ├── typing_engine.*         Per-character state, cursor, backspace
│   ├── metrics.*                WPM / accuracy / consistency formulas
│   ├── typing_screen.*          Live typing test UI
│   ├── results_screen.*         Results + WPM graph UI
│   ├── home_screen.*            Settings/home menu UI
│   ├── dataset_manager.*        Word/quote loading, difficulty filtering
│   ├── database.*               SQLite-backed history & personal bests
│   ├── downloader_win.cpp       Windows networking (WinHTTP)
│   ├── downloader_linux.cpp     Linux networking (libcurl)
│   ├── config_store.*           Persisted user settings
│   ├── dataset_meta.*           Tracks dataset freshness for `update`
│   └── app_paths.*              Resolves paths relative to the executable
├── data/
│   ├── words/common-1k.txt
│   └── quotes/quotes.json
└── tests/
    ├── metrics_test.cpp
    └── typing_engine_test.cpp
```

## Design notes

- **Accuracy counts corrected mistakes.** If you type a wrong character and then backspace to fix it, that keystroke still counts against your accuracy — this matches how most modern typing tests (including Monkeytype) actually behave, and required logging every keystroke rather than just diffing the final string.
- **WPM uses the standard 5-characters-per-word convention.** Net WPM counts only correct characters; raw WPM counts every keystroke including corrected ones.
- **Difficulty is percentile-based**, not a fixed character count — it splits whatever word list is currently loaded into thirds by word length, so it scales sensibly whether you're on the small starter list or a larger downloaded one.
- **Networking is platform-specific by design.** Windows uses WinHTTP (built into the OS, no extra dependency); Linux uses libcurl. Both implement the same `downloadWordList()` interface, so the rest of the app doesn't know or care which one is active.
- **Word list license:** the bundled starter list was composed for this project and carries no licensing restrictions. `dashtype download` can optionally fetch [google-10000-english](https://github.com/first20hours/google-10000-english), which is free for educational/personal use but not unrestricted public domain — check that repo's license before redistributing a build that bundles it directly.


## License

MIT

## Acknowledgements

- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) for terminal UI
- [SQLite](https://sqlite.org) for local storage
- Typing UX inspired by [Monkeytype](https://monkeytype.com) (independent implementation, no shared code or assets)
