#pragma once

#include <drogon/HttpSimpleController.h>
#include <nlohmann/json.hpp>
#include "auth/auth_service.h"

using namespace drogon;

class AuthController : public HttpSimpleController<AuthController> {
public:
    virtual void asyncHandleHttpRequest(const HttpRequestPtr& req,
                                        std::function<void(const HttpResponsePtr&)>&& callback) override;

    PATH_LIST_BEGIN
    PATH_ADD("/api/v1/auth/login", Post);
    PATH_ADD("/api/v1/auth/logout", Post);
    PATH_LIST_END
};
