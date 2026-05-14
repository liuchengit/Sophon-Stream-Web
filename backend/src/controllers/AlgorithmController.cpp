#include "controllers/AlgorithmController.h"
#include "db/repositories.h"
#include "utils/api_helper.h"
#include "utils/json_converter.h"
#include "models/entities.h"
#include <drogon/HttpAppFramework.h>
#include <json/json.h>
#include <nlohmann/json.hpp>

using namespace sophon::web;
using AlgorithmModel = sophon::web::models::Algorithm;

void AlgorithmController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
    auto method = req->method();
    auto path = req->path();

    if (method == Get) {
        if (path == "/api/v1/algorithms") {
            auto page = getQueryParamInt(req, "page", 1);
            auto limit = getQueryParamInt(req, "limit", 20);

            auto items = db::AlgorithmRepository::findAll(page, limit);
            auto total = db::AlgorithmRepository::count();

            nlohmann::json jArr = nlohmann::json::array();
            for (const auto& item : items) {
                nlohmann::json j;
                j["id"] = item.id;
                j["name"] = item.name;
                j["version"] = item.version;
                j["type"] = item.type;
                j["model_path"] = item.model_path;
                j["config_schema"] = item.config_schema;
                j["plugin_path"] = item.plugin_path;
                j["status"] = item.status;
                j["created_at"] = item.created_at;
                jArr.push_back(j);
            }

            callback(successListResponse(toCppJson(jArr), total, page, limit));
            return;
        } else {
            // GET /api/v1/algorithms/{id}
            auto idStr = path.substr(path.find_last_of('/') + 1);
            try {
                int id = std::stoi(idStr);
                auto opt = db::AlgorithmRepository::findById(id);
                if (!opt.has_value()) {
                    callback(errorResponse(404, "Algorithm not found", k404NotFound));
                    return;
                }

                nlohmann::json j;
                j["id"] = opt->id;
                j["name"] = opt->name;
                j["version"] = opt->version;
                j["type"] = opt->type;
                j["model_path"] = opt->model_path;
                j["config_schema"] = opt->config_schema;
                j["plugin_path"] = opt->plugin_path;
                j["status"] = opt->status;
                j["created_at"] = opt->created_at;

                callback(successResponse(toCppJson(j)));
            } catch (...) {
                callback(errorResponse(400, "Invalid ID", k400BadRequest));
            }
            return;
        }
    } else if (method == Post) {
        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            callback(errorResponse(400, "Invalid JSON body"));
            return;
        }

        AlgorithmModel algo;
        algo.name = (*jsonPtr)["name"].asString();
        algo.version = (*jsonPtr)["version"].asString();
        algo.type = (*jsonPtr)["type"].asString();
        algo.model_path = jsonPtr->isMember("model_path") ? (*jsonPtr)["model_path"].asString() : "";
        algo.config_schema = jsonPtr->isMember("config_schema") ? (*jsonPtr)["config_schema"].asString() : "";
        algo.plugin_path = jsonPtr->isMember("plugin_path") ? (*jsonPtr)["plugin_path"].asString() : "";
        algo.status = "inactive";

        int newId = db::AlgorithmRepository::create(algo);

        nlohmann::json j;
        j["id"] = newId;
        j["name"] = algo.name;
        j["version"] = algo.version;
        j["type"] = algo.type;
        j["status"] = algo.status;

        callback(successResponse(toCppJson(j)));
        return;
    } else if (method == Put) {
        auto idStr = path.substr(path.find_last_of('/') + 1);
        int id = std::stoi(idStr);

        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            callback(errorResponse(400, "Invalid JSON body"));
            return;
        }

        auto opt = db::AlgorithmRepository::findById(id);
        if (!opt) {
            callback(errorResponse(404, "Algorithm not found", k404NotFound));
            return;
        }

        AlgorithmModel algo = *opt;
        if (jsonPtr->isMember("name")) algo.name = (*jsonPtr)["name"].asString();
        if (jsonPtr->isMember("version")) algo.version = (*jsonPtr)["version"].asString();
        if (jsonPtr->isMember("type")) algo.type = (*jsonPtr)["type"].asString();
        if (jsonPtr->isMember("model_path")) algo.model_path = (*jsonPtr)["model_path"].asString();
        if (jsonPtr->isMember("config_schema")) algo.config_schema = (*jsonPtr)["config_schema"].asString();
        if (jsonPtr->isMember("plugin_path")) algo.plugin_path = (*jsonPtr)["plugin_path"].asString();
        if (jsonPtr->isMember("status")) algo.status = (*jsonPtr)["status"].asString();

        bool updated = db::AlgorithmRepository::update(id, algo);
        if (!updated) {
            callback(errorResponse(500, "Failed to update", k500InternalServerError));
            return;
        }

        nlohmann::json j;
        j["id"] = id;
        j["name"] = algo.name;
        callback(successResponse(toCppJson(j)));
        return;
    } else if (method == Delete) {
        auto idStr = path.substr(path.find_last_of('/') + 1);
        int id = std::stoi(idStr);

        bool removed = db::AlgorithmRepository::remove(id);
        if (!removed) {
            callback(errorResponse(404, "Algorithm not found", k404NotFound));
            return;
        }

        callback(successResponse(Json::objectValue));
        return;
    }

    callback(errorResponse(405, "Method not allowed", k405MethodNotAllowed));
}
