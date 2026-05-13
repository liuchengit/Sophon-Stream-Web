#include "controllers/DeviceController.h"
#include "types.h"
#include <iostream>
#include <sstream>

using json = nlohmann::json;

void DeviceController::asyncHandleHttpRequest(const HttpRequestPtr& req,
                                              std::function<void(const HttpResponsePtr&)>&& callback) {
    auto method = req->method();
    auto path = req->path();

    if (method == Get && path == "/api/v1/devices") {
        // List devices
        json response = {
            {"code", 0},
            {"message", "success"},
            {"data", {
                {"items", json::array()},
                {"total", 0}
            }}
        };
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);

    } else if (method == Post && path == "/api/v1/devices") {
        // Create device
        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            json error = {{"code", static_cast<int>(sophon::web::ErrorCode::ERR_INVALID_REQUEST)},
                         {"message", "Invalid request body"}};
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }

        json response = {
            {"code", 0},
            {"message", "success"},
            {"data", {
                {"id", 1},
                {"name", (*jsonPtr)["name"]},
                {"type", (*jsonPtr)["type"]},
                {"ip_address", (*jsonPtr)["ip_address"]},
                {"port", (*jsonPtr).value("port", 8080)},
                {"status", "offline"},
                {"model", (*jsonPtr).value("model", "")},
            }}
        };
        auto resp = HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(k201Created);
        callback(resp);

    } else if (method == Get && path.find("/api/v1/devices/") == 0) {
        // Get single device
        std::string idStr = path.substr(path.find_last_of('/') + 1);
        json response = {
            {"code", 0},
            {"message", "success"},
            {"data", {
                {"id", std::stoi(idStr)},
                {"name", "Device 1"},
                {"type", "camera"},
                {"ip_address", "192.168.1.100"},
                {"port", 8080},
                {"status", "online"},
            }}
        };
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);

    } else if (method == Delete && path.find("/api/v1/devices/") == 0) {
        // Delete device
        json response = {{"code", 0}, {"message", "success"}};
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);

    } else {
        json error = {{"code", 404}, {"message", "Not found"}};
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k404NotFound);
        callback(resp);
    }
}
