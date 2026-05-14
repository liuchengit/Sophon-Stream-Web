#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

class PluginController : public HttpSimpleController<PluginController> {
public:
    virtual void asyncHandleHttpRequest(const HttpRequestPtr& req,
                                        std::function<void(const HttpResponsePtr&)>&& callback) override;

    PATH_LIST_BEGIN
    PATH_ADD("/api/v1/plugins", Get);
    PATH_ADD("/api/v1/plugins", Post);
    PATH_ADD("/api/v1/plugins/{id}", Put);
    PATH_ADD("/api/v1/plugins/{id}", Delete);
    PATH_LIST_END
};
