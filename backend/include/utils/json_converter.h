#pragma once

#include <json/json.h>
#include <nlohmann/json.hpp>

inline Json::Value toCppJson(const nlohmann::json& j) {
    if (j.is_null()) return Json::nullValue;
    if (j.is_boolean()) return Json::Value(j.get<bool>());
    if (j.is_number_integer()) return Json::Value(j.get<int64_t>());
    if (j.is_number_float()) return Json::Value(j.get<double>());
    if (j.is_string()) return Json::Value(j.get<std::string>());
    if (j.is_array()) {
        Json::Value arr(Json::arrayValue);
        for (const auto& item : j) arr.append(toCppJson(item));
        return arr;
    }
    if (j.is_object()) {
        Json::Value obj(Json::objectValue);
        for (auto it = j.begin(); it != j.end(); ++it) {
            obj[it.key()] = toCppJson(it.value());
        }
        return obj;
    }
    return Json::nullValue;
}
