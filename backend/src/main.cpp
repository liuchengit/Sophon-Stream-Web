#include <drogon/drogon.h>
#include <iostream>
#include <filesystem>
#include "db/database_manager.h"
#include "auth/auth_service.h"
#include "stream_engine.h"
#include "middleware/auth_middleware.h"
#include "utils/api_helper.h"
#include "utils/json_converter.h"
#include "db/repositories.h"
#include "models/entities.h"
#include <nlohmann/json.hpp>

using namespace drogon;
using namespace sophon::web::middleware;
using namespace sophon::web::models;

static void handleWorkflow(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& callback,
                           const std::string& idStr) {
    using namespace sophon::web;
    using namespace sophon::web::db;

    auto method = req->method();

    if (method == Get) {
        if (idStr.empty()) {
            auto page = getQueryParamInt(req, "page", 1);
            auto limit = getQueryParamInt(req, "limit", 20);
            auto status = req->getParameter("status");

            auto items = WorkflowRepository::findAll(status, page, limit);
            auto total = WorkflowRepository::count(status);

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
        } else {
            try {
                int id = std::stoi(idStr);
                auto opt = WorkflowRepository::findById(id);
                if (!opt) {
                    callback(errorResponse(404, "Workflow not found", k404NotFound));
                    return;
                }

                auto nodes = WorkflowNodeRepository::findByWorkflowId(id);
                auto edges = WorkflowEdgeRepository::findByWorkflowId(id);

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
            } catch (...) {
                callback(errorResponse(400, "Invalid ID", k400BadRequest));
            }
        }
    } else if (method == Post && idStr.empty()) {
        auto jsonPtr = req->getJsonObject();
        if (!jsonPtr) {
            callback(errorResponse(400, "Invalid JSON body"));
            return;
        }

        Workflow wf;
        wf.name = (*jsonPtr)["name"].asString();
        wf.description = jsonPtr->isMember("description") ? (*jsonPtr)["description"].asString() : "";
        wf.status = jsonPtr->isMember("status") ? (*jsonPtr)["status"].asString() : "draft";

        int newId = WorkflowRepository::create(wf);

        nlohmann::json j;
        j["id"] = newId;
        j["name"] = wf.name;
        j["description"] = wf.description;
        j["status"] = wf.status;

        callback(successResponse(toCppJson(j)));
    } else if (method == Put && !idStr.empty()) {
        try {
            int id = std::stoi(idStr);
            auto jsonPtr = req->getJsonObject();
            if (!jsonPtr) {
                callback(errorResponse(400, "Invalid JSON body"));
                return;
            }

            auto opt = WorkflowRepository::findById(id);
            if (!opt) {
                callback(errorResponse(404, "Workflow not found", k404NotFound));
                return;
            }

            Workflow wf = *opt;
            if (jsonPtr->isMember("name")) wf.name = (*jsonPtr)["name"].asString();
            if (jsonPtr->isMember("description")) wf.description = (*jsonPtr)["description"].asString();
            if (jsonPtr->isMember("status")) wf.status = (*jsonPtr)["status"].asString();

            WorkflowRepository::update(id, wf);

            if (jsonPtr->isMember("nodes")) {
                WorkflowNodeRepository::removeByWorkflowId(id);
                for (const auto& jNode : (*jsonPtr)["nodes"]) {
                    WorkflowNode node;
                    node.workflow_id = id;
                    node.node_id = jNode["id"].asString();
                    node.node_type = jNode.isMember("type") ? jNode["type"].asString() : "default";
                    node.position_x = jNode["position"]["x"].asDouble();
                    node.position_y = jNode["position"]["y"].asDouble();
                    if (jNode["data"].isMember("label"))
                        node.label = jNode["data"]["label"].asString();
                    if (jNode["data"].isMember("config"))
                        node.config_json = jNode["data"]["config"].toStyledString();
                    else
                        node.config_json = "{}";
                    WorkflowNodeRepository::create(node);
                }
            }

            if (jsonPtr->isMember("edges")) {
                WorkflowEdgeRepository::removeByWorkflowId(id);
                for (const auto& jEdge : (*jsonPtr)["edges"]) {
                    WorkflowEdge edge;
                    edge.workflow_id = id;
                    edge.edge_id = jEdge["id"].asString();
                    edge.source_node = jEdge["source"].asString();
                    edge.target_node = jEdge["target"].asString();
                    edge.source_handle = jEdge.isMember("sourceHandle") ? jEdge["sourceHandle"].asString() : "default";
                    edge.target_handle = jEdge.isMember("targetHandle") ? jEdge["targetHandle"].asString() : "default";
                    WorkflowEdgeRepository::create(edge);
                }
            }

            nlohmann::json j;
            j["id"] = id;
            j["name"] = wf.name;
            j["status"] = wf.status;
            callback(successResponse(toCppJson(j)));
        } catch (...) {
            callback(errorResponse(400, "Invalid ID", k400BadRequest));
        }
    } else if (method == Delete && !idStr.empty()) {
        try {
            int id = std::stoi(idStr);
            WorkflowNodeRepository::removeByWorkflowId(id);
            WorkflowEdgeRepository::removeByWorkflowId(id);
            bool removed = WorkflowRepository::remove(id);
            if (!removed) {
                callback(errorResponse(404, "Workflow not found", k404NotFound));
                return;
            }
            callback(successResponse(Json::objectValue));
        } catch (...) {
            callback(errorResponse(400, "Invalid ID", k400BadRequest));
        }
    } else {
        callback(errorResponse(405, "Method not allowed", k405MethodNotAllowed));
    }
}

int main() {
    std::cout << "Starting Sophon-Stream Web Management System..." << std::endl;

    // Ensure required directories exist
    std::filesystem::create_directories("logs");
    std::filesystem::create_directories("uploads");
    std::filesystem::create_directories("data");
    std::filesystem::create_directories("engine/configs");

    // Initialize database
    if (!sophon::web::db::DatabaseManager::instance().initialize("data/sophon-web.db")) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }

    // Initialize authentication
    if (!sophon::web::auth::AuthService::instance().initialize()) {
        std::cerr << "Failed to initialize auth service" << std::endl;
        return 1;
    }

    // Initialize sophon-stream engine
    if (!sophon::stream::StreamEngine::instance().initialize("engine/configs/default.json")) {
        std::cerr << "Warning: Failed to initialize sophon-stream engine (running without hardware)" << std::endl;
    }

    // Register middlewares
    drogon::app().registerMiddleware(std::make_shared<AuthMiddleware>());
    drogon::app().registerMiddleware(std::make_shared<RBACMiddleware>());

    // Register workflow handlers
    drogon::app().registerHandler("/api/v1/workflows",
        [](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
            handleWorkflow(req, std::move(callback), "");
        },
        {Get, Post});

    drogon::app().registerHandler("/api/v1/workflows/{id}",
        [](const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, const std::string& id) {
            handleWorkflow(req, std::move(callback), id);
        },
        {Get, Put, Delete, Options});

    // Load configuration
    drogon::app().loadConfigFile("config.json");

    std::cout << "Sophon-Stream Web Management System started successfully." << std::endl;
    std::cout << "API: http://localhost:8080" << std::endl;
    std::cout << "Default admin: admin / admin123" << std::endl;

    // Start the application
    drogon::app().run();

    // Cleanup
    sophon::stream::StreamEngine::instance().shutdown();
    sophon::web::db::DatabaseManager::instance().close();

    return 0;
}
