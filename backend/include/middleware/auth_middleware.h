#pragma once

#include <drogon/HttpMiddleware.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <nlohmann/json.hpp>
#include "auth/auth_service.h"
#include <unordered_map>
#include <unordered_set>

using namespace drogon;

namespace sophon {
namespace web {
namespace middleware {

class AuthMiddleware : public HttpMiddleware<AuthMiddleware, false> {
public:
    void invoke(const HttpRequestPtr& req, MiddlewareNextCallback&& nextCb, MiddlewareCallback&& endCb) override;

private:
    bool isPublicPath(const std::string& path) const;
    std::string extractToken(const HttpRequestPtr& req) const;
};

class RBACMiddleware : public HttpMiddleware<RBACMiddleware, false> {
public:
    void invoke(const HttpRequestPtr& req, MiddlewareNextCallback&& nextCb, MiddlewareCallback&& endCb) override;

    static void registerPermission(const std::string& pathPattern, const std::string& permission);
    static void registerRolePermission(const std::string& role, const std::string& permission);

private:
    static std::unordered_map<std::string, std::string> pathPermissions_;
    static std::unordered_map<std::string, std::unordered_set<std::string>> rolePermissions_;

    std::string getRequiredPermission(const std::string& path) const;
};

} // namespace middleware
} // namespace web
} // namespace sophon
