#include "controllers/MonitoringController.h"
#include "db/repositories.h"
#include "utils/api_helper.h"
#include "utils/json_converter.h"
#include "models/entities.h"
#include <drogon/HttpAppFramework.h>
#include <json/json.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace sophon::web;

static double getCpuUsage() {
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

static double getMemoryUsage() {
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

void MonitoringController::asyncHandleHttpRequest(const HttpRequestPtr& req,
                                                  std::function<void(const HttpResponsePtr&)>&& callback) {
    auto path = req->path();

    if (path == "/api/v1/monitoring/resources") {
        double cpuUsage = getCpuUsage();
        double memoryUsage = getMemoryUsage();
        double tpuUsage = 0.0;

        nlohmann::json j;
        j["cpu_usage"] = cpuUsage;
        j["memory_usage"] = memoryUsage;
        j["memory_total"] = 8192;
        j["memory_used"] = static_cast<int>(8192 * memoryUsage / 100);
        j["tpu_usage"] = tpuUsage;

        callback(successResponse(toCppJson(j)));

    } else if (path == "/api/v1/monitoring/history") {
        auto type = getQueryParam(req, "type", "cpu");
        auto startTime = getQueryParam(req, "startTime");
        auto endTime = getQueryParam(req, "endTime");

        auto metrics = db::MonitoringMetricRepository::findByType(type, startTime, endTime);

        nlohmann::json jArr = nlohmann::json::array();
        for (const auto& metric : metrics) {
            nlohmann::json j;
            j["id"] = metric.id;
            j["metric_type"] = metric.metric_type;
            j["value"] = metric.value;
            j["recorded_at"] = metric.recorded_at;
            jArr.push_back(j);
        }

        callback(successResponse(toCppJson(jArr)));

    } else if (path == "/api/v1/monitoring/stats") {
        int onlineDevices = db::DeviceRepository::count("online", "");
        int offlineDevices = db::DeviceRepository::count("offline", "");
        int totalDevices = onlineDevices + offlineDevices + db::DeviceRepository::count("offline", "tpu") + db::DeviceRepository::count("online", "camera");

        auto allDevices = db::DeviceRepository::findAll();
        totalDevices = allDevices.size();
        onlineDevices = 0;
        offlineDevices = 0;
        for (const auto& d : allDevices) {
            if (d.status == "online") onlineDevices++;
            else offlineDevices++;
        }

        auto allTasks = db::TaskRepository::findAll();
        int totalTasks = allTasks.size();
        int runningTasks = 0;
        int stoppedTasks = 0;
        for (const auto& t : allTasks) {
            if (t.status == "running") runningTasks++;
            else if (t.status == "stopped") stoppedTasks++;
        }

        int totalAlarms = db::AlarmEventRepository::count();

        auto allAlgos = db::AlgorithmRepository::findAll();
        int totalAlgos = allAlgos.size();
        int activeAlgos = 0;
        for (const auto& a : allAlgos) {
            if (a.status == "active") activeAlgos++;
        }

        nlohmann::json j;
        j["devices"]["total"] = totalDevices;
        j["devices"]["online"] = onlineDevices;
        j["devices"]["offline"] = offlineDevices;
        j["tasks"]["total"] = totalTasks;
        j["tasks"]["running"] = runningTasks;
        j["tasks"]["stopped"] = stoppedTasks;
        j["alarms"]["total"] = totalAlarms;
        j["alarms"]["today"] = totalAlarms;
        j["algorithms"]["total"] = totalAlgos;
        j["algorithms"]["active"] = activeAlgos;

        callback(successResponse(toCppJson(j)));

    } else {
        callback(errorResponse(404, "Not found", k404NotFound));
    }
}
