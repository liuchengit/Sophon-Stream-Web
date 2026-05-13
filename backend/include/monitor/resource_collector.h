#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

namespace sophon {
namespace web {

struct ResourceMetrics {
    double tpu_usage = 0.0;
    double cpu_usage = 0.0;
    double memory_usage = 0.0;
    long long memory_total = 0;
    long long memory_used = 0;
    int disk_usage_percent = 0;
    long long timestamp = 0;
};

using MetricsCallback = std::function<void(const ResourceMetrics&)>;

class ResourceCollector {
public:
    static ResourceCollector& instance();

    void start(int intervalSeconds = 5);
    void stop();

    void addCallback(MetricsCallback callback);
    ResourceMetrics getCurrentMetrics();

    bool checkThresholds();
    std::vector<std::string> getThresholdAlerts();

    void setTpuThreshold(double threshold) { tpuThreshold_ = threshold; }
    void setCpuThreshold(double threshold) { cpuThreshold_ = threshold; }
    void setMemoryThreshold(double threshold) { memoryThreshold_ = threshold; }

private:
    ResourceCollector() = default;

    void collectLoop();
    double getCpuUsage();
    double getMemoryUsage();
    double getTpuUsage();
    int getDiskUsage();

    std::atomic<bool> running_{false};
    std::thread collectThread_;
    std::vector<MetricsCallback> callbacks_;
    std::mutex mutex_;

    ResourceMetrics currentMetrics_;

    double tpuThreshold_ = 85.0;
    double cpuThreshold_ = 90.0;
    double memoryThreshold_ = 85.0;
};

} // namespace web
} // namespace sophon
