#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <queue>
#include <chrono>
#include <nlohmann/json.hpp>

namespace sophon {
namespace stream {

using json = nlohmann::json;

struct DetectionResult {
    int classId;
    float confidence;
    int x, y, width, height;
    int trackId;
    std::string timestamp;
};

struct AlarmTrigger {
    int ruleId;
    int taskId;
    std::string reason;
    json context;
    std::string timestamp;
};

using ResultCallback = std::function<void(const DetectionResult&)>;
using AlarmCallback = std::function<void(const AlarmTrigger&)>;

class ResultCollector {
public:
    static ResultCollector& instance();

    void addResultCallback(ResultCallback callback);
    void addAlarmCallback(AlarmCallback callback);

    void collectResult(const DetectionResult& result);
    void checkAlarmCondition(const DetectionResult& result);

    void configureAlarmRule(int ruleId, const std::string& conditionExpr, int debounceSeconds);
    void removeAlarmRule(int ruleId);

    std::vector<DetectionResult> getRecentResults(int limit = 100);
    std::vector<AlarmTrigger> getRecentAlarms(int limit = 50);

    void saveEvidence(const AlarmTrigger& alarm, const std::string& imagePath);

private:
    ResultCollector() = default;

    bool evaluateCondition(const DetectionResult& result, const std::string& expr);
    bool isInDebounceWindow(int ruleId);

    struct AlarmRule {
        int id;
        std::string conditionExpr;
        int debounceSeconds;
        std::chrono::steady_clock::time_point lastTrigger;
    };

    std::vector<ResultCallback> resultCallbacks_;
    std::vector<AlarmCallback> alarmCallbacks_;
    std::map<int, AlarmRule> alarmRules_;

    std::queue<DetectionResult> recentResults_;
    std::queue<AlarmTrigger> recentAlarms_;
    std::mutex mutex_;

    static const int MAX_RESULTS = 1000;
    static const int MAX_ALARMS = 500;
};

} // namespace stream
} // namespace sophon
