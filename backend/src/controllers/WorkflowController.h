#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

class WorkflowController : public HttpSimpleController<WorkflowController> {
public:
    virtual void asyncHandleHttpRequest(const HttpRequestPtr& req,
                                        std::function<void(const HttpResponsePtr&)>&& callback) override;

    PATH_LIST_BEGIN
    PATH_ADD("/api/v1/workflows", Get, Post);
    PATH_ADD("/api/v1/workflows/{id}", Get, Put, Delete, Options);
    PATH_LIST_END
};
