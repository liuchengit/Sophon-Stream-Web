#pragma once

#include <drogon/WebSocketController.h>
#include <drogon/Drogon.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <atomic>
#include <fstream>
#include <sstream>

using namespace drogon;
using json = nlohmann::json;

class MonitorWebSocket : public WebSocketController<MonitorWebSocket> {
public:
    void handleNewMessage(const WebSocketConnectionPtr&, std::string&&, const WebSocketMessageType&) override;
    void handleNewConnection(const HttpRequestPtr&, const WebSocketConnectionPtr&) override;
    void handleConnectionClosed(const WebSocketConnectionPtr&) override;

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/ws/monitoring");
    WS_PATH_LIST_END

private:
    void startMonitoringThread();
    json getResourceMetrics();

    std::atomic<bool> running_{false};
    std::thread monitorThread_;
    std::vector<WebSocketConnectionPtr> clients_;
    std::mutex clientsMutex_;
};

inline void MonitorWebSocket::handleNewMessage(const WebSocketConnectionPtr&, std::string&&, const WebSocketMessageType&) {
    // Client messages are ignored in this implementation
}

inline void MonitorWebSocket::handleNewConnection(const HttpRequestPtr&, const WebSocketConnectionPtr& wsConn) {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    clients_.push_back(wsConn);

    if (!running_) {
        running_ = true;
        startMonitoringThread();
    }
}

inline void MonitorWebSocket::handleConnectionClosed(const WebSocketConnectionPtr& wsConn) {
    std::lock_guard<std::mutex> lock(clientsMutex_);
    clients_.erase(
        std::remove_if(clients_.begin(), clients_.end(),
            [&wsConn](const WebSocketConnectionPtr& conn) { return conn == wsConn; }),
        clients_.end()
    );

    if (clients_.empty()) {
        running_ = false;
    }
}

inline void MonitorWebSocket::startMonitoringThread() {
    monitorThread_ = std::thread([this]() {
        while (running_) {
            auto metrics = getResourceMetrics();
            std::string msg = metrics.dump();

            std::lock_guard<std::mutex> lock(clientsMutex_);
            for (auto it = clients_.begin(); it != clients_.end();) {
                if ((*it)->connected()) {
                    (*it)->send(msg);
                    ++it;
                } else {
                    it = clients_.erase(it);
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });
}

inline json MonitorWebSocket::getResourceMetrics() {
    double cpuUsage = 0.0;
    std::ifstream stat("/proc/stat");
    if (stat.is_open()) {
        std::string line;
        std::getline(stat, line);
        std::istringstream iss(line);
        std::string label;
        iss >> label;
        long long user, nice, system, idle, iowait, irq, softirq, steal;
        iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
        long long totalIdle = idle + iowait;
        long long total = user + nice + system + idle + iowait + irq + softirq + steal;
        if (total > 0) {
            cpuUsage = (1.0 - (double)totalIdle / total) * 100.0;
        }
        stat.close();
    }

    double memoryUsage = 0.0;
    long long memTotal = 8192;
    long long memUsed = 0;
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        long long memAvailable = 0;
        while (std::getline(meminfo, line)) {
            if (line.find("MemTotal:") == 0) {
                std::istringstream iss(line.substr(9));
                iss >> memTotal;
            } else if (line.find("MemAvailable:") == 0) {
                std::istringstream iss(line.substr(13));
                iss >> memAvailable;
            }
        }
        memUsed = memTotal - memAvailable;
        if (memTotal > 0) {
            memoryUsage = (double)memUsed / memTotal * 100.0;
        }
        meminfo.close();
    }

    return {
        {"tpu_usage", 0.0},
        {"cpu_usage", cpuUsage},
        {"memory_usage", memoryUsage},
        {"memory_total", memTotal},
        {"memory_used", memUsed},
        {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
    };
}
