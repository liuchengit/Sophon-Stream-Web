#include "controllers/AuthController.h"
#include "types.h"
#include <iostream>

using json = nlohmann::json;

void AuthController::asyncHandleHttpRequest(const HttpRequestPtr& req,
                                            std::function<void(const HttpResponsePtr&)>&& callback) {
    auto path = req->path();

    if (path == "/api/v1/auth/login") {
        // Parse login request
        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            json error = {{"code", static_cast<int>(sophon::web::ErrorCode::ERR_INVALID_REQUEST)},
                         {"message", "Invalid request body"}};
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }

        std::string username = (*jsonPtr)["username"].asString();
        std::string password = (*jsonPtr)["password"].asString();

        auto token = sophon::web::auth::AuthService::instance().login(username, password);
        if (!token) {
            json error = {{"code", static_cast<int>(sophon::web::ErrorCode::ERR_UNAUTHORIZED)},
                         {"message", "Invalid username or password"}};
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k401Unauthorized);
            callback(resp);
            return;
        }

        json response = {
            {"code", 0},
            {"message", "success"},
            {"data", {
                {"token", *token},
                {"user", {
                    {"id", 1},
                    {"username", username},
                    {"role", "admin"}
                }}
            }}
        };

        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } else if (path == "/api/v1/auth/logout") {
        json response = {{"code", 0}, {"message", "success"}};
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } else {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        callback(resp);
    }
}
