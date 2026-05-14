#include "websocket/websocket_manager.h"
#include <chrono>
#include <iostream>

namespace sophon {
namespace web {
namespace websocket {

WebSocketManager& WebSocketManager::instance() {
    static WebSocketManager instance;
    return instance;
}

void WebSocketManager::addConnection(const std::string& connId, const drogon::WebSocketConnectionPtr& conn) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    connections_[connId] = conn;
}

void WebSocketManager::removeConnection(const std::string& connId) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    connections_.erase(connId);
}

void WebSocketManager::broadcast(const nlohmann::json& message) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::string msg = message.dump();
    for (const auto& [id, conn] : connections_) {
        if (conn->connected()) {
            conn->send(msg);
        }
    }
}

void WebSocketManager::sendTo(const std::string& connId, const nlohmann::json& message) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = connections_.find(connId);
    if (it != connections_.end() && it->second->connected()) {
        it->second->send(message.dump());
    }
}

void WebSocketManager::broadcastAlarm(const std::string& alarmId, const std::string& ruleName, const std::string& level, const std::string& message) {
    nlohmann::json msg;
    msg["type"] = "alarm";
    msg["data"]["id"] = alarmId;
    msg["data"]["ruleName"] = ruleName;
    msg["data"]["level"] = level;
    msg["data"]["message"] = message;
    msg["data"]["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    broadcast(msg);
}

void WebSocketManager::broadcastMetrics(const std::string& metricType, double value, const std::string& timestamp) {
    nlohmann::json msg;
    msg["type"] = "metrics";
    msg["data"]["metricType"] = metricType;
    msg["data"]["value"] = value;
    msg["data"]["timestamp"] = timestamp;
    broadcast(msg);
}

int WebSocketManager::getConnectionCount() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return static_cast<int>(connections_.size());
}

} // namespace websocket
} // namespace web
} // namespace sophon
