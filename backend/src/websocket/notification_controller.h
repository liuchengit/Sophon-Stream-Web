#pragma once

#include <drogon/WebSocketController.h>
#include <drogon/WebSocketConnection.h>
#include <string>

using namespace drogon;

class NotificationController : public WebSocketController<NotificationController> {
public:
    void handleNewMessage(const WebSocketConnectionPtr&, std::string&&, const WebSocketMessageType&) override;
    void handleNewConnection(const HttpRequestPtr&, const WebSocketConnectionPtr&) override;
    void handleConnectionClosed(const WebSocketConnectionPtr&) override;

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/api/v1/ws/notifications");
    WS_PATH_LIST_END
};
