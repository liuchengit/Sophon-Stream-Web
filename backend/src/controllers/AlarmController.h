#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

class AlarmController : public HttpSimpleController<AlarmController> {
public:
    virtual void asyncHandleHttpRequest(const HttpRequestPtr& req,
                                        std::function<void(const HttpResponsePtr&)>&& callback) override;

    PATH_LIST_BEGIN
    PATH_ADD("/api/v1/alarm/rules", Get);
    PATH_ADD("/api/v1/alarm/rules", Post);
    PATH_ADD("/api/v1/alarm/rules/{id}", Get);
    PATH_ADD("/api/v1/alarm/rules/{id}", Put);
    PATH_ADD("/api/v1/alarm/rules/{id}", Delete);
    PATH_ADD("/api/v1/alarm/rules/{id}/toggle", Post);
    PATH_ADD("/api/v1/alarm/events", Get);
    PATH_LIST_END
};
