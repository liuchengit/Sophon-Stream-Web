#include "monitor/resource_collector.h"
#include "utils/logger.h"
#include "db/repositories.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <filesystem>

namespace sophon {
namespace web {

ResourceCollector& ResourceCollector::instance() {
    static ResourceCollector instance;
    return instance;
}

void ResourceCollector::start(int intervalSeconds) {
    if (running_) return;

    running_ = true;
    collectThread_ = std::thread([this, intervalSeconds]() {
        collectLoop();
    });

    LOG_INFO("Monitor", "Resource collector started (interval: " + std::to_string(intervalSeconds) + "s)");
}

void ResourceCollector::stop() {
    if (!running_) return;

    running_ = false;
    if (collectThread_.joinable()) {
        collectThread_.join();
    }

    LOG_INFO("Monitor", "Resource collector stopped");
}

void ResourceCollector::addCallback(MetricsCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.push_back(std::move(callback));
}

ResourceMetrics ResourceCollector::getCurrentMetrics() {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentMetrics_;
}

bool ResourceCollector::checkThresholds() {
    std::lock_guard<std::mutex> lock(mutex_);

    bool alert = false;
    if (currentMetrics_.tpu_usage > tpuThreshold_) alert = true;
    if (currentMetrics_.cpu_usage > cpuThreshold_) alert = true;
    if (currentMetrics_.memory_usage > memoryThreshold_) alert = true;

    return alert;
}

std::vector<std::string> ResourceCollector::getThresholdAlerts() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> alerts;

    if (currentMetrics_.tpu_usage > tpuThreshold_) {
        alerts.push_back("TPU usage exceeds " + std::to_string((int)tpuThreshold_) + "% (current: " + std::to_string((int)currentMetrics_.tpu_usage) + "%)");
    }
    if (currentMetrics_.cpu_usage > cpuThreshold_) {
        alerts.push_back("CPU usage exceeds " + std::to_string((int)cpuThreshold_) + "% (current: " + std::to_string((int)currentMetrics_.cpu_usage) + "%)");
    }
    if (currentMetrics_.memory_usage > memoryThreshold_) {
        alerts.push_back("Memory usage exceeds " + std::to_string((int)memoryThreshold_) + "% (current: " + std::to_string((int)currentMetrics_.memory_usage) + "%)");
    }

    return alerts;
}

void ResourceCollector::collectLoop() {
    while (running_) {
        auto metrics = getCurrentMetrics();
        metrics.tpu_usage = getTpuUsage();
        metrics.cpu_usage = getCpuUsage();
        metrics.memory_usage = getMemoryUsage();
        metrics.memory_total = 8192; // In production: read from /proc/meminfo
        metrics.memory_used = (long long)(metrics.memory_total * metrics.memory_usage / 100);
        metrics.disk_usage_percent = getDiskUsage();
        metrics.timestamp = std::chrono::system_clock::now().time_since_epoch().count();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            currentMetrics_ = metrics;
        }

        // Persist to database
        models::MonitoringMetric tpuMetric;
        tpuMetric.metric_type = "tpu";
        tpuMetric.value = metrics.tpu_usage;
        db::MonitoringMetricRepository::create(tpuMetric);

        models::MonitoringMetric cpuMetric;
        cpuMetric.metric_type = "cpu";
        cpuMetric.value = metrics.cpu_usage;
        db::MonitoringMetricRepository::create(cpuMetric);

        models::MonitoringMetric memMetric;
        memMetric.metric_type = "memory";
        memMetric.value = metrics.memory_usage;
        db::MonitoringMetricRepository::create(memMetric);

        // Notify callbacks
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& cb : callbacks_) {
                cb(metrics);
            }
        }

        // Check thresholds
        if (checkThresholds()) {
            auto alerts = getThresholdAlerts();
            for (const auto& alert : alerts) {
                LOG_WARNING("Monitor", alert);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

double ResourceCollector::getCpuUsage() {
    std::ifstream stat("/proc/stat");
    if (!stat.is_open()) return 0.0;

    std::string line;
    std::getline(stat, line);
    stat.close();

    std::istringstream iss(line);
    std::string label;
    iss >> label;

    long long user, nice, system, idle, iowait, irq, softirq, steal;
    iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    long long totalIdle = idle + iowait;
    long long total = user + nice + system + idle + iowait + irq + softirq + steal;

    if (total == 0) return 0.0;
    return (1.0 - (double)totalIdle / total) * 100.0;
}

double ResourceCollector::getMemoryUsage() {
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo.is_open()) return 0.0;

    std::string line;
    long long memTotal = 0, memAvailable = 0;

    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream iss(line.substr(9));
            iss >> memTotal;
        } else if (line.find("MemAvailable:") == 0) {
            std::istringstream iss(line.substr(13));
            iss >> memAvailable;
        }
    }
    meminfo.close();

    if (memTotal == 0) return 0.0;
    return (1.0 - (double)memAvailable / memTotal) * 100.0;
}

double ResourceCollector::getTpuUsage() {
    // In production: read from Sophgo BM interface
    // For now, return simulated value
    return 0.0;
}

int ResourceCollector::getDiskUsage() {
    try {
        auto space = std::filesystem::space(".");
        if (space.capacity == 0) return 0;
        return (int)((space.capacity - space.available) * 100 / space.capacity);
    } catch (...) {
        return 0;
    }
}

} // namespace web
} // namespace sophon
