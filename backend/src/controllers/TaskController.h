#pragma once

#include <drogon/HttpSimpleController.h>
#include <nlohmann/json.hpp>

using namespace drogon;

class TaskController : public HttpSimpleController<TaskController> {
public:
    virtual void asyncHandleHttpRequest(const HttpRequestPtr& req,
                                        std::function<void(const HttpResponsePtr&)>&& callback) override;

    PATH_LIST_BEGIN
    PATH_ADD("/api/v1/tasks", Get);
    PATH_ADD("/api/v1/tasks", Post);
    PATH_ADD("/api/v1/tasks/{id}", Get);
    PATH_ADD("/api/v1/tasks/{id}", Put);
    PATH_ADD("/api/v1/tasks/{id}", Delete);
    PATH_ADD("/api/v1/tasks/{id}/start", Post);
    PATH_ADD("/api/v1/tasks/{id}/stop", Post);
    PATH_ADD("/api/v1/tasks/{id}/pause", Post);
    PATH_ADD("/api/v1/tasks/{id}/resume", Post);
    PATH_ADD("/api/v1/tasks/{id}/config", Put);
    PATH_LIST_END
};
