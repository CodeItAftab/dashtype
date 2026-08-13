#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>

struct sqlite3;

struct TestRecord {
    std::string timestamp;
    int durationSeconds = 0;
    double wpm = 0.0;
    double rawWpm = 0.0;
    double accuracyPercent = 0.0;
    double consistencyPercent = 0.0;
    int correctChars = 0;
    int errors = 0;
    std::string mode;
    std::string difficulty;
};

struct StatsSummary {
    int totalTests = 0;
    double avgAccuracy = 0.0;
    std::map<int, std::optional<double>> bestByDuration;  // keyed by duration
};

class Database {
public:
    Database() = default;
    ~Database();

    bool open(const std::string& path);

    bool recordTest(const TestRecord& record);
    std::optional<double> personalBest(int durationSeconds);
    std::vector<TestRecord> recentHistory(int limit);
    StatsSummary overallStats();

private:
    sqlite3* db_ = nullptr;
};