#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

class ConfigController : public HttpSimpleController<ConfigController> {
public:
    virtual void asyncHandleHttpRequest(const HttpRequestPtr& req,
                                        std::function<void(const HttpResponsePtr&)>&& callback) override;

    PATH_LIST_BEGIN
    PATH_ADD("/api/v1/config", Get);
    PATH_ADD("/api/v1/config/{key}", Get);
    PATH_ADD("/api/v1/config/{key}", Put);
    PATH_ADD("/api/v1/config/{key}/history", Get);
    PATH_LIST_END
};
