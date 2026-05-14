#include "controllers/TaskController.h"
#include "db/repositories.h"
#include "utils/api_helper.h"
#include "utils/json_converter.h"
#include "models/entities.h"
#include <drogon/HttpAppFramework.h>
#include <json/json.h>
#include <nlohmann/json.hpp>

using namespace sophon::web;

static nlohmann::json taskToJson(const models::Task& t) {
    nlohmann::json j;
    j["id"] = t.id;
    j["name"] = t.name;
    j["description"] = t.description;
    j["device_id"] = t.device_id;
    j["graph_config"] = t.graph_config;
    j["status"] = t.status;
    j["schedule_cron"] = t.schedule_cron;
    j["created_at"] = t.created_at;
    j["updated_at"] = t.updated_at;
    return j;
}

void TaskController::asyncHandleHttpRequest(const HttpRequestPtr& req,
                                            std::function<void(const HttpResponsePtr&)>&& callback) {
    auto method = req->method();
    auto path = req->path();

    if (method == Get && path == "/api/v1/tasks") {
        auto status = getQueryParam(req, "status");
        auto page = getQueryParamInt(req, "page", 1);
        auto limit = getQueryParamInt(req, "limit", 20);

        auto tasks = db::TaskRepository::findAll(status, page, limit);
        auto total = db::TaskRepository::count(status);

        nlohmann::json jArr = nlohmann::json::array();
        for (const auto& task : tasks) {
            jArr.push_back(taskToJson(task));
        }

        callback(successListResponse(toCppJson(jArr), total, page, limit));

    } else if (method == Post && path == "/api/v1/tasks") {
        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            callback(errorResponse(400, "Invalid JSON body"));
            return;
        }

        models::Task task;
        task.name = (*jsonPtr)["name"].asString();
        if (jsonPtr->isMember("description")) task.description = (*jsonPtr)["description"].asString();
        if (jsonPtr->isMember("device_id")) task.device_id = (*jsonPtr)["device_id"].asInt();
        if (jsonPtr->isMember("graph_config")) task.graph_config = (*jsonPtr)["graph_config"].asString();
        task.status = "stopped";
        if (jsonPtr->isMember("schedule_cron")) task.schedule_cron = (*jsonPtr)["schedule_cron"].asString();

        int newId = db::TaskRepository::create(task);

        nlohmann::json j = taskToJson(task);
        j["id"] = newId;

        auto resp = successResponse(toCppJson(j));
        resp->setStatusCode(k201Created);
        callback(resp);

    } else if (path.find("/api/v1/tasks/") == 0) {
        auto lastSlash = path.find_last_of('/');
        auto idStr = path.substr(path.find("tasks/") + 6, lastSlash - path.find("tasks/") - 6);
        int id = std::stoi(idStr);
        std::string action = path.substr(lastSlash + 1);

        if (action.empty()) {
            if (method == Get) {
                auto opt = db::TaskRepository::findById(id);
                if (!opt.has_value()) {
                    callback(errorResponse(404, "Task not found", k404NotFound));
                    return;
                }
                callback(successResponse(toCppJson(taskToJson(*opt))));

            } else if (method == Put) {
                auto opt = db::TaskRepository::findById(id);
                if (!opt.has_value()) {
                    callback(errorResponse(404, "Task not found", k404NotFound));
                    return;
                }

                auto jsonPtr = req->getJsonObject();
                if (!jsonPtr) {
                    callback(errorResponse(400, "Invalid JSON body"));
                    return;
                }

                models::Task task = *opt;
                if (jsonPtr->isMember("name")) task.name = (*jsonPtr)["name"].asString();
                if (jsonPtr->isMember("description")) task.description = (*jsonPtr)["description"].asString();
                if (jsonPtr->isMember("device_id")) task.device_id = (*jsonPtr)["device_id"].asInt();
                if (jsonPtr->isMember("graph_config")) task.graph_config = (*jsonPtr)["graph_config"].asString();
                if (jsonPtr->isMember("status")) task.status = (*jsonPtr)["status"].asString();
                if (jsonPtr->isMember("schedule_cron")) task.schedule_cron = (*jsonPtr)["schedule_cron"].asString();

                bool updated = db::TaskRepository::update(id, task);
                if (!updated) {
                    callback(errorResponse(500, "Failed to update task", k500InternalServerError));
                    return;
                }

                callback(successResponse(toCppJson(taskToJson(task))));

            } else if (method == Delete) {
                bool removed = db::TaskRepository::remove(id);
                if (!removed) {
                    callback(errorResponse(404, "Task not found", k404NotFound));
                    return;
                }
                callback(successResponse(Json::objectValue));

            } else {
                callback(errorResponse(405, "Method not allowed", k405MethodNotAllowed));
            }

        } else if (action == "start" && method == Post) {
            auto opt = db::TaskRepository::findById(id);
            if (!opt.has_value()) {
                callback(errorResponse(404, "Task not found", k404NotFound));
                return;
            }

            bool updated = db::TaskRepository::updateStatus(id, "running");
            if (!updated) {
                callback(errorResponse(500, "Failed to start task", k500InternalServerError));
                return;
            }

            nlohmann::json j;
            j["id"] = id;
            j["status"] = "running";
            callback(successResponse(toCppJson(j)));

        } else if (action == "stop" && method == Post) {
            auto opt = db::TaskRepository::findById(id);
            if (!opt.has_value()) {
                callback(errorResponse(404, "Task not found", k404NotFound));
                return;
            }

            bool updated = db::TaskRepository::updateStatus(id, "stopped");
            if (!updated) {
                callback(errorResponse(500, "Failed to stop task", k500InternalServerError));
                return;
            }

            nlohmann::json j;
            j["id"] = id;
            j["status"] = "stopped";
            callback(successResponse(toCppJson(j)));

        } else {
            callback(errorResponse(404, "Not found", k404NotFound));
        }

    } else {
        callback(errorResponse(404, "Not found", k404NotFound));
    }
}
