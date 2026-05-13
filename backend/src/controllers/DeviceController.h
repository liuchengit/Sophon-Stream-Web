#pragma once

#include <drogon/HttpSimpleController.h>
#include <nlohmann/json.hpp>

using namespace drogon;

class DeviceController : public HttpSimpleController<DeviceController> {
public:
    virtual void asyncHandleHttpRequest(const HttpRequestPtr& req,
                                        std::function<void(const HttpResponsePtr&)>&& callback) override;

    PATH_LIST_BEGIN
    PATH_ADD("/api/v1/devices", Get);
    PATH_ADD("/api/v1/devices", Post);
    PATH_ADD("/api/v1/devices/{id}", Get);
    PATH_ADD("/api/v1/devices/{id}", Put);
    PATH_ADD("/api/v1/devices/{id}", Delete);
    PATH_LIST_END
};
