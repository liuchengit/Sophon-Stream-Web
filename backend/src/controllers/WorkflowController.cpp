#include "controllers/WorkflowController.h"
#include "db/repositories.h"
#include "utils/api_helper.h"
#include "utils/json_converter.h"
#include "models/entities.h"
#include <drogon/HttpAppFramework.h>
#include <json/json.h>
#include <nlohmann/json.hpp>

using namespace drogon;
using namespace sophon::web;
using namespace sophon::web::models;

static std::string safeJsonStr(const Json::Value& v, const std::string& def = "") {
    return v.isString() ? v.asString() : def;
}

static double safeJsonDouble(const Json::Value& v, double def = 0.0) {
    return v.isNumeric() ? v.asDouble() : def;
}

void WorkflowController::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
    auto method = req->method();
    auto path = req->path();

    // GET /api/v1/workflows - list all workflows
    if (method == Get && path == "/api/v1/workflows") {
        auto page = getQueryParamInt(req, "page", 1);
        auto limit = getQueryParamInt(req, "limit", 20);
        auto status = req->getParameter("status");

        auto items = db::WorkflowRepository::findAll(status, page, limit);
        auto total = db::WorkflowRepository::count(status);

        nlohmann::json jArr = nlohmann::json::array();
        for (const auto& item : items) {
            nlohmann::json j;
            j["id"] = item.id;
            j["name"] = item.name;
            j["description"] = item.description;
            j["status"] = item.status;
            j["created_at"] = item.created_at;
            j["updated_at"] = item.updated_at;
            jArr.push_back(j);
        }

        callback(successListResponse(toCppJson(jArr), total, page, limit));
        return;
    }

    // POST /api/v1/workflows - create workflow
    if (method == Post && path == "/api/v1/workflows") {
        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            callback(errorResponse(400, "Invalid JSON body"));
            return;
        }

        Workflow wf;
        wf.name = (*jsonPtr)["name"].asString();
        wf.description = jsonPtr->isMember("description") ? (*jsonPtr)["description"].asString() : "";
        wf.status = jsonPtr->isMember("status") ? (*jsonPtr)["status"].asString() : "draft";

        int newId = db::WorkflowRepository::create(wf);

        nlohmann::json j;
        j["id"] = newId;
        j["name"] = wf.name;
        j["description"] = wf.description;
        j["status"] = wf.status;

        callback(successResponse(toCppJson(j)));
        return;
    }

    // Parse workflow ID from path
    std::string basePath = "/api/v1/workflows/";
    if (path.find(basePath) == 0 && path.size() > basePath.size()) {
        std::string rest = path.substr(basePath.size());

        // Remove trailing slash
        if (!rest.empty() && rest.back() == '/') rest.pop_back();

        // Extract ID
        std::string idStr;
        std::string action;
        auto slashPos = rest.find('/');
        if (slashPos != std::string::npos) {
            idStr = rest.substr(0, slashPos);
            action = rest.substr(slashPos + 1);
        } else {
            idStr = rest;
        }

        try {
            int id = std::stoi(idStr);

            // GET /api/v1/workflows/{id} - get workflow with nodes and edges
            if (method == Get && action.empty()) {
                auto opt = db::WorkflowRepository::findById(id);
                if (!opt) {
                    callback(errorResponse(404, "Workflow not found", k404NotFound));
                    return;
                }

                auto nodes = db::WorkflowNodeRepository::findByWorkflowId(id);
                auto edges = db::WorkflowEdgeRepository::findByWorkflowId(id);

                nlohmann::json j;
                j["id"] = opt->id;
                j["name"] = opt->name;
                j["description"] = opt->description;
                j["status"] = opt->status;
                j["created_at"] = opt->created_at;
                j["updated_at"] = opt->updated_at;

                nlohmann::json jNodes = nlohmann::json::array();
                for (const auto& n : nodes) {
                    nlohmann::json node;
                    node["id"] = n.node_id;
                    node["type"] = n.node_type;
                    node["position"]["x"] = n.position_x;
                    node["position"]["y"] = n.position_y;
                    node["data"]["label"] = n.label;
                    node["data"]["config"] = nlohmann::json::parse(n.config_json, nullptr, false);
                    jNodes.push_back(node);
                }

                nlohmann::json jEdges = nlohmann::json::array();
                for (const auto& e : edges) {
                    nlohmann::json edge;
                    edge["id"] = e.edge_id;
                    edge["source"] = e.source_node;
                    edge["target"] = e.target_node;
                    edge["sourceHandle"] = e.source_handle;
                    edge["targetHandle"] = e.target_handle;
                    jEdges.push_back(edge);
                }

                j["nodes"] = jNodes;
                j["edges"] = jEdges;

                callback(successResponse(toCppJson(j)));
                return;
            }

            // PUT /api/v1/workflows/{id} - update workflow
            if (method == Put && action.empty()) {
                auto jsonPtr = req->getJsonObject();
                if (!jsonPtr) {
                    callback(errorResponse(400, "Invalid JSON body"));
                    return;
                }

                auto opt = db::WorkflowRepository::findById(id);
                if (!opt) {
                    callback(errorResponse(404, "Workflow not found", k404NotFound));
                    return;
                }

                Workflow wf = *opt;
                if (jsonPtr->isMember("name")) wf.name = (*jsonPtr)["name"].asString();
                if (jsonPtr->isMember("description")) wf.description = (*jsonPtr)["description"].asString();
                if (jsonPtr->isMember("status")) wf.status = (*jsonPtr)["status"].asString();

                db::WorkflowRepository::update(id, wf);

                // Handle nodes if provided
                if (jsonPtr->isMember("nodes")) {
                    db::WorkflowNodeRepository::removeByWorkflowId(id);
                    for (const auto& jNode : (*jsonPtr)["nodes"]) {
                        WorkflowNode node;
                        node.workflow_id = id;
                        node.node_id = safeJsonStr(jNode["id"]);
                        node.node_type = safeJsonStr(jNode["type"], "default");
                        node.position_x = safeJsonDouble(jNode["position"]["x"]);
                        node.position_y = safeJsonDouble(jNode["position"]["y"]);
                        if (jNode["data"].isMember("label"))
                            node.label = jNode["data"]["label"].asString();
                        if (jNode["data"].isMember("config"))
                            node.config_json = jNode["data"]["config"].toStyledString();
                        else
                            node.config_json = "{}";
                        db::WorkflowNodeRepository::create(node);
                    }
                }

                // Handle edges if provided
                if (jsonPtr->isMember("edges")) {
                    db::WorkflowEdgeRepository::removeByWorkflowId(id);
                    for (const auto& jEdge : (*jsonPtr)["edges"]) {
                        WorkflowEdge edge;
                        edge.workflow_id = id;
                        edge.edge_id = safeJsonStr(jEdge["id"]);
                        edge.source_node = safeJsonStr(jEdge["source"]);
                        edge.target_node = safeJsonStr(jEdge["target"]);
                        edge.source_handle = jEdge.isMember("sourceHandle") ? jEdge["sourceHandle"].asString() : "default";
                        edge.target_handle = jEdge.isMember("targetHandle") ? jEdge["targetHandle"].asString() : "default";
                        db::WorkflowEdgeRepository::create(edge);
                    }
                }

                nlohmann::json j;
                j["id"] = id;
                j["name"] = wf.name;
                j["status"] = wf.status;
                callback(successResponse(toCppJson(j)));
                return;
            }

            // DELETE /api/v1/workflows/{id}
            if (method == Delete && action.empty()) {
                db::WorkflowNodeRepository::removeByWorkflowId(id);
                db::WorkflowEdgeRepository::removeByWorkflowId(id);
                bool removed = db::WorkflowRepository::remove(id);
                if (!removed) {
                    callback(errorResponse(404, "Workflow not found", k404NotFound));
                    return;
                }
                callback(successResponse(Json::objectValue));
                return;
            }

            // OPTIONS /api/v1/workflows/{id}
            if (method == Options) {
                Json::Value resp;
                callback(HttpResponse::newHttpJsonResponse(resp));
                return;
            }
        } catch (...) {
            callback(errorResponse(400, "Invalid ID", k400BadRequest));
            return;
        }
    }

    callback(errorResponse(404, "Not found", k404NotFound));
}
