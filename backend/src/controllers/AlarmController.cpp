#include "controllers/AlarmController.h"
#include "db/repositories.h"
#include "utils/api_helper.h"
#include "utils/json_converter.h"
#include <json/json.h>
#include <nlohmann/json.hpp>

using namespace drogon;
using namespace sophon::web;
using AlarmRuleModel = sophon::web::models::AlarmRule;

void AlarmController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
    auto method = req->method();
    auto path = req->path();

    // GET /api/v1/alarm/events
    if (method == Get && path.find("/alarm/events") != std::string::npos) {
        int page = getQueryParamInt(req, "page", 1);
        int limit = getQueryParamInt(req, "limit", 20);

        auto events = db::AlarmEventRepository::findAll(page, limit);
        int total = db::AlarmEventRepository::count();

        Json::Value items(Json::arrayValue);
        for (const auto& e : events) {
            Json::Value item;
            item["id"] = e.id;
            item["rule_id"] = e.rule_id;
            item["task_id"] = e.task_id;
            item["evidence_path"] = e.evidence_path;
            item["context"] = e.context;
            item["triggered_at"] = e.triggered_at;
            items.append(item);
        }
        callback(successListResponse(items, total, page, limit));
        return;
    }

    // POST /api/v1/alarm/rules/{id}/toggle
    if (method == Post && path.find("/toggle") != std::string::npos) {
        std::string idStr = path.substr(path.find("/alarm/rules/") + 13);
        size_t slashPos = idStr.find('/');
        if (slashPos != std::string::npos) idStr = idStr.substr(0, slashPos);
        int id = std::stoi(idStr);

        auto opt = db::AlarmRuleRepository::findById(id);
        if (!opt) {
            callback(errorResponse(404, "Alarm rule not found", k404NotFound));
            return;
        }

        db::AlarmRuleRepository::toggle(id, !opt->enabled);
        Json::Value data;
        data["id"] = id;
        data["enabled"] = !opt->enabled;
        callback(successResponse(data));
        return;
    }

    // POST /api/v1/alarm/rules
    if (method == Post) {
        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            callback(errorResponse(400, "Invalid JSON body"));
            return;
        }

        AlarmRuleModel rule;
        rule.name = (*jsonPtr)["name"].asString();
        rule.condition_expr = (*jsonPtr)["condition_expr"].asString();
        rule.debounce_seconds = jsonPtr->isMember("debounce_seconds") ? (*jsonPtr)["debounce_seconds"].asInt() : 0;
        rule.notification_channels = jsonPtr->isMember("notification_channels") ? (*jsonPtr)["notification_channels"].asString() : "webhook";
        rule.enabled = jsonPtr->isMember("enabled") ? (*jsonPtr)["enabled"].asBool() : true;

        int newId = db::AlarmRuleRepository::create(rule);
        Json::Value data;
        data["id"] = newId;
        data["name"] = rule.name;
        data["enabled"] = rule.enabled;
        callback(successResponse(data));
        return;
    }

    // GET /api/v1/alarm/rules
    if (method == Get) {
        auto rules = db::AlarmRuleRepository::findAll(false);
        Json::Value items(Json::arrayValue);
        for (const auto& r : rules) {
            Json::Value item;
            item["id"] = r.id;
            item["name"] = r.name;
            item["condition_expr"] = r.condition_expr;
            item["debounce_seconds"] = r.debounce_seconds;
            item["notification_channels"] = r.notification_channels;
            item["enabled"] = r.enabled;
            item["created_at"] = r.created_at;
            items.append(item);
        }
        callback(successListResponse(items, static_cast<int>(rules.size()), 1, static_cast<int>(rules.size())));
        return;
    }

    // PUT/DELETE /api/v1/alarm/rules/{id}
    if (path.find("/alarm/rules/") != std::string::npos && path.back() != '/') {
        std::string idStr = path.substr(path.find_last_of('/') + 1);
        if (idStr == "toggle") return;
        int id = std::stoi(idStr);

        if (method == Put) {
            auto jsonPtr = req->getJsonObject();
            if (!jsonPtr) {
                callback(errorResponse(400, "Invalid JSON body"));
                return;
            }

            auto opt = db::AlarmRuleRepository::findById(id);
            if (!opt) {
                callback(errorResponse(404, "Alarm rule not found", k404NotFound));
                return;
            }

            AlarmRuleModel rule = *opt;
            if (jsonPtr->isMember("name")) rule.name = (*jsonPtr)["name"].asString();
            if (jsonPtr->isMember("condition_expr")) rule.condition_expr = (*jsonPtr)["condition_expr"].asString();
            if (jsonPtr->isMember("debounce_seconds")) rule.debounce_seconds = (*jsonPtr)["debounce_seconds"].asInt();
            if (jsonPtr->isMember("notification_channels")) rule.notification_channels = (*jsonPtr)["notification_channels"].asString();
            if (jsonPtr->isMember("enabled")) rule.enabled = (*jsonPtr)["enabled"].asBool();

            db::AlarmRuleRepository::update(id, rule);
            Json::Value data;
            data["id"] = id;
            data["name"] = rule.name;
            callback(successResponse(data));
            return;
        }

        if (method == Delete) {
            bool removed = db::AlarmRuleRepository::remove(id);
            if (!removed) {
                callback(errorResponse(404, "Alarm rule not found", k404NotFound));
                return;
            }
            callback(successResponse(Json::objectValue));
            return;
        }
    }

    callback(errorResponse(405, "Method not allowed", k405MethodNotAllowed));
}
