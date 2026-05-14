#include "controllers/MonitoringController.h"
#include "types.h"
#include <utils/json_converter.h>
#include <iostream>
#include <fstream>

using json = nlohmann::json;

// Read CPU usage from /proc/stat
static double getCpuUsage() {
    std::ifstream stat("/proc/stat");
    if (!stat.is_open()) return 0.0;

    std::string line;
    std::getline(stat, line);
    stat.close();

    // Parse: cpu  user nice system idle iowait irq softirq steal guest guest_nice
    std::istringstream iss(line);
    std::string label;
    iss >> label; // "cpu"

    long long user, nice, system, idle, iowait, irq, softirq, steal;
    iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    long long totalIdle = idle + iowait;
    long long total = user + nice + system + idle + iowait + irq + softirq + steal;

    if (total == 0) return 0.0;
    return (1.0 - (double)totalIdle / total) * 100.0;
}

// Read memory usage from /proc/meminfo
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

        // TPU usage simulation (in production, read from BM interface)
        double tpuUsage = 0.0;

        json response = {
            {"code", 0},
            {"message", "success"},
            {"data", {
                {"tpu_usage", tpuUsage},
                {"cpu_usage", cpuUsage},
                {"memory_usage", memoryUsage},
                {"memory_total", 8192},
                {"memory_used", static_cast<int>(8192 * memoryUsage / 100)},
            }}
        };
        auto resp = HttpResponse::newHttpJsonResponse(toCppJson(response));
        callback(resp);

    } else if (path == "/api/v1/monitoring/history") {
        json response = {
            {"code", 0},
            {"message", "success"},
            {"data", json::array()}
        };
        auto resp = HttpResponse::newHttpJsonResponse(toCppJson(response));
        callback(resp);

    } else {
        json error = {{"code", 404}, {"message", "Not found"}};
        auto resp = HttpResponse::newHttpJsonResponse(toCppJson(error));
        resp->setStatusCode(k404NotFound);
        callback(resp);
    }
}
