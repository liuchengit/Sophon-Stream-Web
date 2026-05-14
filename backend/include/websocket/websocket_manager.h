#pragma once

#include <drogon/WebSocketController.h>
#include <drogon/WebSocketConnection.h>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace sophon {
namespace web {
namespace websocket {

class WebSocketManager {
public:
    static WebSocketManager& instance();

    void addConnection(const std::string& connId, const drogon::WebSocketConnectionPtr& conn);
    void removeConnection(const std::string& connId);
    void broadcast(const nlohmann::json& message);
    void sendTo(const std::string& connId, const nlohmann::json& message);
    void broadcastAlarm(const std::string& alarmId, const std::string& ruleName, const std::string& level, const std::string& message);
    void broadcastMetrics(const std::string& metricType, double value, const std::string& timestamp);
    int getConnectionCount() const;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, drogon::WebSocketConnectionPtr> connections_;
};

} // namespace websocket
} // namespace web
} // namespace sophon
