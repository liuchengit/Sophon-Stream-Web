#include "controllers/DeviceController.h"
#include "db/repositories.h"
#include "utils/api_helper.h"
#include "utils/json_converter.h"
#include "models/entities.h"
#include <drogon/HttpAppFramework.h>
#include <json/json.h>
#include <nlohmann/json.hpp>

using namespace sophon::web;

static nlohmann::json deviceToJson(const models::Device& d) {
    nlohmann::json j;
    j["id"] = d.id;
    j["name"] = d.name;
    j["type"] = d.type;
    j["ip_address"] = d.ip_address;
    j["port"] = d.port;
    j["status"] = d.status;
    j["model"] = d.model;
    j["firmware_version"] = d.firmware_version;
    j["created_at"] = d.created_at;
    j["updated_at"] = d.updated_at;
    return j;
}

void DeviceController::asyncHandleHttpRequest(const HttpRequestPtr& req,
                                              std::function<void(const HttpResponsePtr&)>&& callback) {
    auto method = req->method();
    auto path = req->path();

    if (method == Get && path == "/api/v1/devices") {
        auto status = getQueryParam(req, "status");
        auto type = getQueryParam(req, "type");
        auto page = getQueryParamInt(req, "page", 1);
        auto limit = getQueryParamInt(req, "limit", 20);

        auto devices = db::DeviceRepository::findAll(status, type, page, limit);
        auto total = db::DeviceRepository::count(status, type);

        nlohmann::json jArr = nlohmann::json::array();
        for (const auto& device : devices) {
            jArr.push_back(deviceToJson(device));
        }

        callback(successListResponse(toCppJson(jArr), total, page, limit));

    } else if (method == Post && path == "/api/v1/devices") {
        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            callback(errorResponse(400, "Invalid JSON body"));
            return;
        }

        models::Device device;
        device.name = (*jsonPtr)["name"].asString();
        device.type = (*jsonPtr)["type"].asString();
        device.ip_address = (*jsonPtr)["ip_address"].asString();
        device.port = (*jsonPtr)["port"].asInt();
        device.status = "offline";
        if (jsonPtr->isMember("model")) device.model = (*jsonPtr)["model"].asString();
        if (jsonPtr->isMember("firmware_version")) device.firmware_version = (*jsonPtr)["firmware_version"].asString();

        int newId = db::DeviceRepository::create(device);

        nlohmann::json j = deviceToJson(device);
        j["id"] = newId;

        auto resp = successResponse(toCppJson(j));
        resp->setStatusCode(k201Created);
        callback(resp);

    } else if (path.find("/api/v1/devices/") == 0) {
        auto idStr = path.substr(path.find_last_of('/') + 1);
        int id = std::stoi(idStr);

        if (method == Get) {
            auto opt = db::DeviceRepository::findById(id);
            if (!opt.has_value()) {
                callback(errorResponse(404, "Device not found", k404NotFound));
                return;
            }
            callback(successResponse(toCppJson(deviceToJson(*opt))));

        } else if (method == Put) {
            auto opt = db::DeviceRepository::findById(id);
            if (!opt.has_value()) {
                callback(errorResponse(404, "Device not found", k404NotFound));
                return;
            }

            auto jsonPtr = req->getJsonObject();
            if (!jsonPtr) {
                callback(errorResponse(400, "Invalid JSON body"));
                return;
            }

            models::Device device = *opt;
            if (jsonPtr->isMember("name")) device.name = (*jsonPtr)["name"].asString();
            if (jsonPtr->isMember("type")) device.type = (*jsonPtr)["type"].asString();
            if (jsonPtr->isMember("ip_address")) device.ip_address = (*jsonPtr)["ip_address"].asString();
            if (jsonPtr->isMember("port")) device.port = (*jsonPtr)["port"].asInt();
            if (jsonPtr->isMember("status")) device.status = (*jsonPtr)["status"].asString();
            if (jsonPtr->isMember("model")) device.model = (*jsonPtr)["model"].asString();
            if (jsonPtr->isMember("firmware_version")) device.firmware_version = (*jsonPtr)["firmware_version"].asString();

            bool updated = db::DeviceRepository::update(id, device);
            if (!updated) {
                callback(errorResponse(500, "Failed to update device", k500InternalServerError));
                return;
            }

            callback(successResponse(toCppJson(deviceToJson(device))));

        } else if (method == Delete) {
            bool removed = db::DeviceRepository::remove(id);
            if (!removed) {
                callback(errorResponse(404, "Device not found", k404NotFound));
                return;
            }
            callback(successResponse(Json::objectValue));

        } else {
            callback(errorResponse(405, "Method not allowed", k405MethodNotAllowed));
        }

    } else {
        callback(errorResponse(404, "Not found", k404NotFound));
    }
}
