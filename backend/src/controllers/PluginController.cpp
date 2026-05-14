#include "controllers/PluginController.h"
#include "db/repositories.h"
#include "utils/api_helper.h"
#include "utils/json_converter.h"
#include <json/json.h>
#include <nlohmann/json.hpp>

using namespace drogon;
using namespace sophon::web;

using PluginModel = sophon::web::models::Plugin;
using AlarmRuleModel = sophon::web::models::AlarmRule;

void PluginController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
    auto method = req->method();
    auto path = req->path();

    // GET /api/v1/plugins
    if (method == Get && (path == "/api/v1/plugins" || path.back() != '/')) {
        std::string status = getQueryParam(req, "status");
        auto plugins = db::PluginRepository::findAll(status);
        Json::Value items(Json::arrayValue);
        for (const auto& p : plugins) {
            Json::Value item;
            item["id"] = p.id;
            item["name"] = p.name;
            item["version"] = p.version;
            item["path"] = p.path;
            item["signature"] = p.signature;
            item["status"] = p.status;
            item["created_at"] = p.created_at;
            items.append(item);
        }
        callback(successListResponse(items, static_cast<int>(plugins.size()), 1, static_cast<int>(plugins.size())));
        return;
    }

    // POST /api/v1/plugins
    if (method == Post) {
        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            callback(errorResponse(400, "Invalid JSON body"));
            return;
        }

        std::string name = (*jsonPtr)["name"].asString();
        auto existing = db::PluginRepository::findByName(name);
        if (existing) {
            callback(errorResponse(409, "Plugin already exists", k409Conflict));
            return;
        }

        PluginModel plugin;
        plugin.name = name;
        plugin.version = (*jsonPtr)["version"].asString();
        plugin.path = (*jsonPtr)["path"].asString();
        plugin.signature = jsonPtr->isMember("signature") ? (*jsonPtr)["signature"].asString() : "";
        plugin.status = "inactive";

        int newId = db::PluginRepository::create(plugin);
        Json::Value data;
        data["id"] = newId;
        data["name"] = plugin.name;
        callback(successResponse(data));
        return;
    }

    // PUT/DELETE /api/v1/plugins/{id}
    if (path.find("/api/v1/plugins/") != std::string::npos && path.back() != '/') {
        std::string idStr = path.substr(path.find_last_of('/') + 1);
        int id = std::stoi(idStr);

        if (method == Put) {
            auto jsonPtr = req->getJsonObject();
            if (!jsonPtr) {
                callback(errorResponse(400, "Invalid JSON body"));
                return;
            }

            auto opt = db::PluginRepository::findById(id);
            if (!opt) {
                callback(errorResponse(404, "Plugin not found", k404NotFound));
                return;
            }

            PluginModel plugin = *opt;
            if (jsonPtr->isMember("version")) plugin.version = (*jsonPtr)["version"].asString();
            if (jsonPtr->isMember("path")) plugin.path = (*jsonPtr)["path"].asString();
            if (jsonPtr->isMember("status")) plugin.status = (*jsonPtr)["status"].asString();

            db::PluginRepository::update(id, plugin);
            Json::Value data;
            data["id"] = id;
            data["status"] = plugin.status;
            callback(successResponse(data));
            return;
        }

        if (method == Delete) {
            bool removed = db::PluginRepository::remove(id);
            if (!removed) {
                callback(errorResponse(404, "Plugin not found", k404NotFound));
                return;
            }
            callback(successResponse(Json::objectValue));
            return;
        }
    }

    callback(errorResponse(405, "Method not allowed", k405MethodNotAllowed));
}
