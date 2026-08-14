#include "database.hpp"
#include <sqlite3.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace {
std::string currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tmBuf;
#ifdef _WIN32
    localtime_s(&tmBuf, &t);
#else
    localtime_r(&t, &tmBuf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M");
    return oss.str();
}
}  // namespace

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

bool Database::open(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        db_ = nullptr;
        return false;
    }
    const char* schema =
        "CREATE TABLE IF NOT EXISTS tests ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp TEXT NOT NULL,"
        "duration_seconds INTEGER NOT NULL,"
        "wpm REAL NOT NULL,"
        "raw_wpm REAL NOT NULL,"
        "accuracy REAL NOT NULL,"
        "consistency REAL NOT NULL,"
        "correct_chars INTEGER NOT NULL,"
        "errors INTEGER NOT NULL,"
        "mode TEXT NOT NULL,"
        "difficulty TEXT NOT NULL"
        ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, schema, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::recordTest(const TestRecord& r) {
    if (!db_) return false;
    const char* sql =
        "INSERT INTO tests (timestamp, duration_seconds, wpm, raw_wpm, accuracy, "
        "consistency, correct_chars, errors, mode, difficulty) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;

    std::string ts = currentTimestamp();
    sqlite3_bind_text(stmt, 1, ts.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, r.durationSeconds);
    sqlite3_bind_double(stmt, 3, r.wpm);
    sqlite3_bind_double(stmt, 4, r.rawWpm);
    sqlite3_bind_double(stmt, 5, r.accuracyPercent);
    sqlite3_bind_double(stmt, 6, r.consistencyPercent);
    sqlite3_bind_int(stmt, 7, r.correctChars);
    sqlite3_bind_int(stmt, 8, r.errors);
    sqlite3_bind_text(stmt, 9, r.mode.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, r.difficulty.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

std::optional<double> Database::personalBest(int durationSeconds) {
    if (!db_) return std::nullopt;
    const char* sql = "SELECT MAX(wpm) FROM tests WHERE duration_seconds = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int(stmt, 1, durationSeconds);

    std::optional<double> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            result = sqlite3_column_double(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<TestRecord> Database::recentHistory(int limit) {
    std::vector<TestRecord> records;
    if (!db_) return records;
    const char* sql =
        "SELECT timestamp, duration_seconds, wpm, raw_wpm, accuracy, consistency, "
        "correct_chars, errors, mode, difficulty FROM tests "
        "ORDER BY id DESC LIMIT ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return records;
    sqlite3_bind_int(stmt, 1, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        TestRecord r;
        r.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        r.durationSeconds = sqlite3_column_int(stmt, 1);
        r.wpm = sqlite3_column_double(stmt, 2);
        r.rawWpm = sqlite3_column_double(stmt, 3);
        r.accuracyPercent = sqlite3_column_double(stmt, 4);
        r.consistencyPercent = sqlite3_column_double(stmt, 5);
        r.correctChars = sqlite3_column_int(stmt, 6);
        r.errors = sqlite3_column_int(stmt, 7);
        r.mode = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        r.difficulty = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        records.push_back(r);
    }
    sqlite3_finalize(stmt);
    return records;
}

StatsSummary Database::overallStats() {
    StatsSummary stats;
    if (!db_) return stats;

    const char* sql = "SELECT COUNT(*), AVG(accuracy) FROM tests;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.totalTests = sqlite3_column_int(stmt, 0);
            if (sqlite3_column_type(stmt, 1) != SQLITE_NULL) {
                stats.avgAccuracy = sqlite3_column_double(stmt, 1);
            }
        }
        sqlite3_finalize(stmt);
    }

    for (int duration : {15, 30, 60}) {
        stats.bestByDuration[duration] = personalBest(duration);
    }

    return stats;
}