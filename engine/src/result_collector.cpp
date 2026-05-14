#include "result_collector.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace sophon {
namespace stream {

static std::string currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

ResultCollector& ResultCollector::instance() {
    static ResultCollector instance;
    return instance;
}

void ResultCollector::addResultCallback(ResultCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    resultCallbacks_.push_back(std::move(callback));
}

void ResultCollector::addAlarmCallback(AlarmCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    alarmCallbacks_.push_back(std::move(callback));
}

void ResultCollector::collectResult(const DetectionResult& result) {
    std::lock_guard<std::mutex> lock(mutex_);

    recentResults_.push(result);
    if (recentResults_.size() > MAX_RESULTS) {
        recentResults_.pop();
    }

    // Notify callbacks
    auto callbacks = resultCallbacks_;
    mutex_.unlock();
    for (auto& cb : callbacks) {
        cb(result);
    }
    mutex_.lock();
}

void ResultCollector::checkAlarmCondition(const DetectionResult& result) {
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& pair : alarmRules_) {
        const auto& rule = pair.second;

        if (isInDebounceWindow(rule.id)) continue;

        if (evaluateCondition(result, rule.conditionExpr)) {
            AlarmTrigger alarm;
            alarm.ruleId = rule.id;
            alarm.taskId = 0;
            alarm.reason = "Condition matched: " + rule.conditionExpr;
            alarm.timestamp = currentTimestamp();

            recentAlarms_.push(alarm);
            if (recentAlarms_.size() > MAX_ALARMS) {
                recentAlarms_.pop();
            }

            alarmRules_.at(pair.first).lastTrigger = std::chrono::steady_clock::now();

            auto callbacks = alarmCallbacks_;
            mutex_.unlock();
            for (auto& cb : callbacks) {
                cb(alarm);
            }
            mutex_.lock();
        }
    }
}

void ResultCollector::configureAlarmRule(int ruleId, const std::string& conditionExpr, int debounceSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);

    AlarmRule rule;
    rule.id = ruleId;
    rule.conditionExpr = conditionExpr;
    rule.debounceSeconds = debounceSeconds;
    rule.lastTrigger = std::chrono::steady_clock::time_point();

    alarmRules_[ruleId] = rule;
}

void ResultCollector::removeAlarmRule(int ruleId) {
    std::lock_guard<std::mutex> lock(mutex_);
    alarmRules_.erase(ruleId);
}

std::vector<DetectionResult> ResultCollector::getRecentResults(int limit) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<DetectionResult> results;

    std::vector<DetectionResult> temp;
    while (!recentResults_.empty()) {
        temp.push_back(recentResults_.front());
        recentResults_.pop();
    }

    for (int i = std::max(0, (int)temp.size() - limit); i < (int)temp.size(); i++) {
        results.push_back(temp[i]);
        recentResults_.push(temp[i]);
    }

    return results;
}

std::vector<AlarmTrigger> ResultCollector::getRecentAlarms(int limit) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AlarmTrigger> alarms;

    std::vector<AlarmTrigger> temp;
    while (!recentAlarms_.empty()) {
        temp.push_back(recentAlarms_.front());
        recentAlarms_.pop();
    }

    for (int i = std::max(0, (int)temp.size() - limit); i < (int)temp.size(); i++) {
        alarms.push_back(temp[i]);
        recentAlarms_.push(temp[i]);
    }

    return alarms;
}

void ResultCollector::saveEvidence(const AlarmTrigger& alarm, const std::string& imagePath) {
    std::lock_guard<std::mutex> lock(mutex_);
    // In production: save frame image to storage
    std::cout << "Evidence saved: " << imagePath << " for alarm " << alarm.ruleId << std::endl;
}

bool ResultCollector::evaluateCondition(const DetectionResult& result, const std::string& expr) {
    // Simple condition evaluation (in production use proper expression parser)
    if (expr.find("confidence>") != std::string::npos) {
        float threshold = std::stof(expr.substr(expr.find(">") + 1));
        return result.confidence > threshold;
    }
    if (expr.find("class==") != std::string::npos) {
        int classId = std::stoi(expr.substr(expr.find("==") + 2));
        return result.classId == classId;
    }
    return false;
}

bool ResultCollector::isInDebounceWindow(int ruleId) {
    auto it = alarmRules_.find(ruleId);
    if (it == alarmRules_.end()) return false;

    if (it->second.debounceSeconds <= 0) return false;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.lastTrigger).count();
    return elapsed < it->second.debounceSeconds;
}

} // namespace stream
} // namespace sophon
