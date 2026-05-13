#pragma once

#include <drogon/HttpSimpleController.h>
#include <nlohmann/json.hpp>

using namespace drogon;

class MonitoringController : public HttpSimpleController<MonitoringController> {
public:
    virtual void asyncHandleHttpRequest(const HttpRequestPtr& req,
                                        std::function<void(const HttpResponsePtr&)>&& callback) override;

    PATH_LIST_BEGIN
    PATH_ADD("/api/v1/monitoring/resources", Get);
    PATH_ADD("/api/v1/monitoring/history", Get);
    PATH_LIST_END
};
