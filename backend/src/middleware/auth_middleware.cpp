#include "middleware/auth_middleware.h"
#include "types.h"
#include <utils/json_converter.h>
#include <iostream>
#include <sstream>

using json = nlohmann::json;

namespace sophon {
namespace web {
namespace middleware {

std::unordered_map<std::string, std::string> RBACMiddleware::pathPermissions_;
std::unordered_map<std::string, std::unordered_set<std::string>> RBACMiddleware::rolePermissions_;

void AuthMiddleware::invoke(const HttpRequestPtr& req, MiddlewareNextCallback&& nextCb, MiddlewareCallback&& endCb) {
    auto path = req->path();

    if (isPublicPath(path)) {
        nextCb([endCb](const HttpResponsePtr& resp) {
            endCb(resp);
        });
        return;
    }

    std::string token = extractToken(req);
    if (token.empty()) {
        json error = {{"code", static_cast<int>(sophon::web::ErrorCode::ERR_UNAUTHORIZED)},
                     {"message", "Missing authentication token"}};
        auto resp = HttpResponse::newHttpJsonResponse(toCppJson(error));
        resp->setStatusCode(k401Unauthorized);
        endCb(resp);
        return;
    }

    if (!auth::AuthService::instance().verifyToken(token)) {
        json error = {{"code", static_cast<int>(sophon::web::ErrorCode::ERR_UNAUTHORIZED)},
                     {"message", "Invalid or expired token"}};
        auto resp = HttpResponse::newHttpJsonResponse(toCppJson(error));
        resp->setStatusCode(k401Unauthorized);
        endCb(resp);
        return;
    }

    nextCb([endCb](const HttpResponsePtr& resp) {
        endCb(resp);
    });
}

bool AuthMiddleware::isPublicPath(const std::string& path) const {
    if (path == "/api/v1/auth/login") return true;
    if (path.find("/api/v1/auth/refresh") == 0) return true;
    if (path.find("/ws/") == 0) return true;

    // Static files are public
    if (path.find(".js") != std::string::npos || path.find(".css") != std::string::npos ||
        path.find(".png") != std::string::npos || path.find(".ico") != std::string::npos ||
        path.find(".html") != std::string::npos) return true;

    return false;
}

std::string AuthMiddleware::extractToken(const HttpRequestPtr& req) const {
    auto authHeader = req->getHeader("Authorization");
    if (authHeader.empty()) return "";

    if (authHeader.substr(0, 7) == "Bearer ") {
        return authHeader.substr(7);
    }
    return "";
}

void RBACMiddleware::registerPermission(const std::string& pathPattern, const std::string& permission) {
    pathPermissions_[pathPattern] = permission;
}

void RBACMiddleware::registerRolePermission(const std::string& role, const std::string& permission) {
    rolePermissions_[role].insert(permission);
}

void RBACMiddleware::invoke(const HttpRequestPtr& req, MiddlewareNextCallback&& nextCb, MiddlewareCallback&& endCb) {
    auto path = req->path();
    auto token = req->getHeader("Authorization");

    if (token.empty()) {
        nextCb([endCb](const HttpResponsePtr& resp) { endCb(resp); });
        return;
    }

    auto userInfo = auth::AuthService::instance().getUserInfo(token.substr(7));
    if (!userInfo) {
        nextCb([endCb](const HttpResponsePtr& resp) { endCb(resp); });
        return;
    }

    std::string requiredPermission = getRequiredPermission(path);
    if (!requiredPermission.empty()) {
        if (!auth::AuthService::instance().hasPermission(token.substr(7), requiredPermission)) {
            json error = {{"code", static_cast<int>(sophon::web::ErrorCode::ERR_FORBIDDEN)},
                         {"message", "Insufficient permissions"}};
            auto resp = HttpResponse::newHttpJsonResponse(toCppJson(error));
            resp->setStatusCode(k403Forbidden);
            endCb(resp);
            return;
        }
    }

    nextCb([endCb](const HttpResponsePtr& resp) {
        endCb(resp);
    });
}

std::string RBACMiddleware::getRequiredPermission(const std::string& path) const {
    // Map paths to permissions
    if (path.find("/api/v1/devices") == 0) {
        return path.find("/devices") == 0 ? "device:write" : "device:read";
    }
    if (path.find("/api/v1/tasks") == 0) {
        if (path.find("/start") != std::string::npos || path.find("/stop") != std::string::npos) {
            return "task:execute";
        }
        return "task:write";
    }
    if (path.find("/api/v1/algorithms") == 0) return "algorithm:read";
    if (path.find("/api/v1/monitoring") == 0) return "monitor:read";
    if (path.find("/api/v1/alarms") == 0) return "alarm:read";
    if (path.find("/api/v1/config") == 0) return "config:read";
    if (path.find("/api/v1/upgrade") == 0) return "upgrade:execute";
    if (path.find("/api/v1/plugins") == 0) return "plugin:read";

    return "";
}

} // namespace middleware
} // namespace web
} // namespace sophon
