#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

class AlgorithmController : public HttpSimpleController<AlgorithmController> {
public:
    virtual void asyncHandleHttpRequest(const HttpRequestPtr& req,
                                        std::function<void(const HttpResponsePtr&)>&& callback) override;

    PATH_LIST_BEGIN
    PATH_ADD("/api/v1/algorithms", Get);
    PATH_ADD("/api/v1/algorithms", Post);
    PATH_ADD("/api/v1/algorithms/{id}", Get);
    PATH_ADD("/api/v1/algorithms/{id}", Put);
    PATH_ADD("/api/v1/algorithms/{id}", Delete);
    PATH_LIST_END
};
