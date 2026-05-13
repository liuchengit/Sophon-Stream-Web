#pragma once

#include <string>
#include <cstdint>

namespace sophon {
namespace web {

enum class ErrorCode : int {
    SUCCESS = 0,
    ERR_INVALID_REQUEST = 400,
    ERR_UNAUTHORIZED = 401,
    ERR_FORBIDDEN = 403,
    ERR_NOT_FOUND = 404,
    ERR_CONFLICT = 409,
    ERR_INTERNAL_ERROR = 500,
    ERR_SERVICE_UNAVAILABLE = 503,

    // Business error codes
    ERR_DEVICE_OFFLINE = 1001,
    ERR_TASK_ALREADY_RUNNING = 1002,
    ERR_GRAPH_CREATE_FAILED = 1003,
    ERR_CONFIG_UPDATE_FAILED = 1004,
    ERR_PLUGIN_LOAD_FAILED = 1005,
    ERR_UPGRADE_FAILED = 1006,
    ERR_ROI_OUT_OF_BOUNDS = 1007,
    ERR_DAG_HAS_CYCLE = 1008,
    ERR_MODEL_NOT_FOUND = 1009,
    ERR_SIGNATURE_VERIFY_FAILED = 1010,
};

struct ApiResponse {
    int code;
    std::string message;
    std::string data;

    static ApiResponse success(const std::string& data = "") {
        return {0, "success", data};
    }

    static ApiResponse error(ErrorCode code, const std::string& message) {
        return {static_cast<int>(code), message, ""};
    }
};

struct PaginationParams {
    int page = 1;
    int limit = 20;
};

struct PaginationResult {
    std::string items;
    int total = 0;
};

} // namespace web
} // namespace sophon
