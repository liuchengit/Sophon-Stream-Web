#include "controllers/ConfigController.h"
#include "db/repositories.h"
#include "utils/api_helper.h"
#include "utils/json_converter.h"
#include <json/json.h>
#include <nlohmann/json.hpp>

using namespace drogon;
using namespace sophon::web;
using namespace sophon::web::models;

void ConfigController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
    auto method = req->method();
    auto path = req->path();

    // GET /api/v1/config - list all configs
    if (method == Get && path == "/api/v1/config") {
        Json::Value items(Json::arrayValue);
        std::vector<std::string> keys = {"system.name", "system.timezone", "system.language", "alarm.webhook_url", "alarm.email_to"};
        for (const auto& key : keys) {
            auto opt = db::ConfigVersionRepository::findLatest(key);
            if (opt) {
                Json::Value item;
                item["key"] = opt->config_key;
                item["value"] = opt->config_value;
                item["version"] = opt->version;
                item["created_at"] = opt->created_at;
                items.append(item);
            }
        }
        callback(successListResponse(items, static_cast<int>(keys.size()), 1, static_cast<int>(keys.size())));
        return;
    }

    // GET /api/v1/config/{key}/history
    if (method == Get && path.find("/history") != std::string::npos) {
        std::string keyPart = path.substr(14);
        size_t histPos = keyPart.find("/history");
        std::string key = keyPart.substr(0, histPos);

        auto history = db::ConfigVersionRepository::findHistory(key, 10);
        Json::Value items(Json::arrayValue);
        for (const auto& h : history) {
            Json::Value item;
            item["key"] = h.config_key;
            item["value"] = h.config_value;
            item["version"] = h.version;
            item["created_at"] = h.created_at;
            items.append(item);
        }
        callback(successListResponse(items, static_cast<int>(history.size()), 1, static_cast<int>(history.size())));
        return;
    }

    // GET /api/v1/config/{key}
    if (method == Get && path.find("/api/v1/config/") != std::string::npos) {
        std::string key = path.substr(14);
        auto opt = db::ConfigVersionRepository::findLatest(key);
        if (!opt) {
            callback(errorResponse(404, "Config not found", k404NotFound));
            return;
        }
        Json::Value data;
        data["key"] = opt->config_key;
        data["value"] = opt->config_value;
        data["version"] = opt->version;
        callback(successResponse(data));
        return;
    }

    // PUT /api/v1/config/{key}
    if (method == Put) {
        std::string key = path.substr(14);
        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            callback(errorResponse(400, "Invalid JSON body"));
            return;
        }

        std::string value = (*jsonPtr)["value"].asString();
        db::ConfigVersionRepository::create(key, value);

        Json::Value data;
        data["key"] = key;
        data["value"] = value;
        callback(successResponse(data));
        return;
    }

    callback(errorResponse(405, "Method not allowed", k405MethodNotAllowed));
}
