#include "metrics.hpp"
#include <cmath>
#include <numeric>


namespace {
    double wpmFromChars(int chars, double durationSeconds){
        if(durationSeconds <= 0.0) return 0;
        double minutes = durationSeconds / 60.0;
        return (static_cast<double>(chars) / 5.0) / minutes;
    }
}

ResultMetrics computeResultMetrics(const TypingEngine& engine, double durationSeconds, const std::vector<WpmSample>& samples){
    ResultMetrics result;
    result.durationSeconds = durationSeconds;
    // --- Count final character states across the target text ---
    int correct = 0;
    int incorrect = 0;

    /*
        We don't know the target length directly form the TypingEngine's public API
        beyond stateAt(), so we can until we hit an out-of-range Untyped that we can distinguish via cursorPos() / extraChars() instead.
    */

    std::size_t scanLimit = engine.cursorPos();
    for(std::size_t i = 0; i < scanLimit; ++i){
        CharState s = engine.stateAt(i);
        if(s == CharState::Correct) ++correct;
        else if(s == CharState::Incorrect) ++incorrect;
    }

    int extraCount = static_cast<int>(engine.extraChars().size());

    const auto& log = engine.keystrokeLog();
    int totalKeystrokes = static_cast<int>(log.size());
    int wrongKeystrokes = 0;
    for(const auto& tc: log){
        if(!tc.wasCorrect) ++wrongKeystrokes;
    }

    result.correctChars = correct;
    result.incorrectChars = incorrect;
    result.totalChars = correct + incorrect;
    result.errors = wrongKeystrokes;
    
    result.wpm = wpmFromChars(correct, durationSeconds);
    result.rawWpm = wpmFromChars(totalKeystrokes, durationSeconds);

    result.accuracyPercent = (totalKeystrokes > 0) ? (100.0 * (totalKeystrokes - wrongKeystrokes) / totalKeystrokes) : 0.0;

    // --- Consistency ---
    if(samples.size() >= 2){
        std::vector<double> intervalWpm;
        for(std::size_t i = 1; i< samples.size(); ++i){
            double dtSeconds = samples[i].secondsElapsed - samples[i - 1].secondsElapsed;
            int dChars = samples[i].correctCharsSoFar - samples[i - 1].correctCharsSoFar;
            if(dtSeconds > 0.0){
                intervalWpm.push_back(wpmFromChars(dChars,dtSeconds));
            }
        }

        if(!intervalWpm.empty()){
            double mean = std::accumulate(intervalWpm.begin(), intervalWpm.end(), 0.0) / intervalWpm.size();

            if(mean > 0.0){
                double sqSum = 0.0;
                for(double w: intervalWpm){
                    sqSum += (w - mean) * (w - mean);
                }
                double stddev = std::sqrt(sqSum / intervalWpm.size());
                double consistency = 100.0 * (1 - (stddev / mean));
                if(consistency < 0.0) consistency = 0.0;
                if(consistency > 100.0) consistency = 100.0;
                result.consistencyPercent = consistency;
            }
        }
    }

    return result;

}