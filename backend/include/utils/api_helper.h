#pragma once

#include <drogon/HttpResponse.h>
#include <drogon/HttpRequest.h>
#include <json/json.h>
#include <utils/json_converter.h>

using namespace drogon;

inline HttpResponsePtr successResponse(const Json::Value& data = Json::objectValue) {
    Json::Value resp;
    resp["code"] = 0;
    resp["message"] = "success";
    resp["data"] = data;
    auto r = HttpResponse::newHttpJsonResponse(resp);
    r->setContentTypeCode(CT_APPLICATION_JSON);
    return r;
}

inline HttpResponsePtr successListResponse(const Json::Value& items, int total, int page = 1, int limit = 20) {
    Json::Value data;
    data["items"] = items;
    data["total"] = total;
    data["page"] = page;
    data["limit"] = limit;
    return successResponse(data);
}

inline HttpResponsePtr errorResponse(int code, const std::string& message, int httpStatus = k400BadRequest) {
    Json::Value resp;
    resp["code"] = code;
    resp["message"] = message;
    auto r = HttpResponse::newHttpJsonResponse(resp);
    r->setStatusCode(static_cast<HttpStatusCode>(httpStatus));
    r->setContentTypeCode(CT_APPLICATION_JSON);
    return r;
}

inline std::string getQueryParam(const HttpRequestPtr& req, const std::string& key, const std::string& defaultVal = "") {
    auto params = req->getParameters();
    auto it = params.find(key);
    return it != params.end() ? it->second : defaultVal;
}

inline int getQueryParamInt(const HttpRequestPtr& req, const std::string& key, int defaultVal = 1) {
    auto val = getQueryParam(req, key);
    return val.empty() ? defaultVal : std::stoi(val);
}
