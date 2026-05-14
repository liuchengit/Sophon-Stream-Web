#include "websocket/notification_controller.h"
#include "websocket/websocket_manager.h"
#include "auth/auth_service.h"
#include <iostream>
#include <sstream>
#include <atomic>
#include <chrono>

using namespace drogon;
using namespace sophon::web::websocket;

static std::string generateConnectionId() {
    static std::atomic<int> counter{0};
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    return "conn-" + std::to_string(ms) + "-" + std::to_string(counter.fetch_add(1));
}

void NotificationController::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string&& message, const WebSocketMessageType& type) {
    if (type == WebSocketMessageType::Text) {
        try {
            auto json = nlohmann::json::parse(message);
            std::string action = json.value("action", "");

            if (action == "ping") {
                nlohmann::json resp;
                resp["type"] = "pong";
                resp["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                wsConnPtr->send(resp.dump());
            } else if (action == "subscribe") {
                auto connIdPtr = wsConnPtr->getContext<std::string>();
                if (connIdPtr && json.contains("channels")) {
                    for (const auto& ch : json["channels"]) {
                        nlohmann::json subResp;
                        subResp["type"] = "subscribed";
                        subResp["channel"] = ch.get<std::string>();
                        wsConnPtr->send(subResp.dump());
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "WebSocket message parse error: " << e.what() << std::endl;
        }
    }
}

void NotificationController::handleNewConnection(const HttpRequestPtr& req, const WebSocketConnectionPtr& conn) {
    // Extract token from query param
    std::string token;
    
    // Try query parameter
    auto params = req->getParameters();
    auto it = params.find("token");
    if (it != params.end()) {
        token = it->second;
    }
    
    // Try Authorization header
    if (token.empty()) {
        token = req->getHeader("Authorization");
        if (token.size() > 7 && token.substr(0, 7) == "Bearer ") {
            token = token.substr(7);
        }
    }

    // Try query from URL
    if (token.empty()) {
        auto query = req->query();
        auto pos = query.find("token=");
        if (pos != std::string::npos) {
            auto start = pos + 6;
            auto end = query.find('&', start);
            if (end == std::string::npos) end = query.size();
            token = query.substr(start, end - start);
        }
    }

    std::cout << "WebSocket connection attempt. Token present: " << (!token.empty()) << std::endl;

    if (!token.empty() && sophon::web::auth::AuthService::instance().verifyToken(token)) {
        std::string connId = generateConnectionId();
        conn->setContext(std::make_shared<std::string>(connId));
        WebSocketManager::instance().addConnection(connId, conn);

        nlohmann::json welcome;
        welcome["type"] = "connected";
        welcome["connectionId"] = connId;
        welcome["message"] = "WebSocket connected";
        conn->send(welcome.dump());

        std::cout << "WebSocket connected: " << connId << " (total: " 
                  << WebSocketManager::instance().getConnectionCount() << ")" << std::endl;
    } else {
        conn->send(R"({"type":"error","message":"Authentication required"})");
        conn->shutdown();
    }
}

void NotificationController::handleConnectionClosed(const WebSocketConnectionPtr& conn) {
    auto connIdPtr = conn->getContext<std::string>();
    if (connIdPtr && !connIdPtr->empty()) {
        WebSocketManager::instance().removeConnection(*connIdPtr);
        std::cout << "WebSocket disconnected: " << *connIdPtr << " (total: "
                  << WebSocketManager::instance().getConnectionCount() << ")" << std::endl;
    }
}
