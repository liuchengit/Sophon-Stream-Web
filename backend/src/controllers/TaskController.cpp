#include "controllers/TaskController.h"
#include "types.h"
#include <iostream>

using json = nlohmann::json;

void TaskController::asyncHandleHttpRequest(const HttpRequestPtr& req,
                                            std::function<void(const HttpResponsePtr&)>&& callback) {
    auto method = req->method();
    auto path = req->path();

    if (method == Get && path == "/api/v1/tasks") {
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

    } else if (method == Post && path == "/api/v1/tasks") {
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
                {"status", "stopped"},
            }}
        };
        auto resp = HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(k201Created);
        callback(resp);

    } else if (path.find("/api/v1/tasks/") == 0) {
        auto pos = path.find_last_of('/');
        std::string idStr = path.substr(path.find("tasks/") + 6, pos - path.find("tasks/") - 6);
        std::string action = path.substr(pos + 1);

        if (method == Get && action.empty()) {
            json response = {
                {"code", 0},
                {"message", "success"},
                {"data", {
                    {"id", std::stoi(idStr)},
                    {"name", "Task 1"},
                    {"status", "stopped"},
                }}
            };
            auto resp = HttpResponse::newHttpJsonResponse(response);
            callback(resp);
        } else if (action == "start" || action == "stop" || action == "pause" || action == "resume") {
            json response = {
                {"code", 0},
                {"message", "success"},
                {"data", {
                    {"id", std::stoi(idStr)},
                    {"status", action == "start" ? "running" : "stopped"},
                }}
            };
            auto resp = HttpResponse::newHttpJsonResponse(response);
            callback(resp);
        } else if (method == Delete) {
            json response = {{"code", 0}, {"message", "success"}};
            auto resp = HttpResponse::newHttpJsonResponse(response);
            callback(resp);
        } else {
            json error = {{"code", 404}, {"message", "Not found"}};
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound);
            callback(resp);
        }
    } else {
        json error = {{"code", 404}, {"message", "Not found"}};
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k404NotFound);
        callback(resp);
    }
}
