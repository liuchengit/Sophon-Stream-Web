#pragma once

#include <string>
#include <cstdint>
#include <optional>

namespace sophon {
namespace web {
namespace models {

struct User {
    int id = 0;
    std::string username;
    std::string password_hash;
    std::string email;
    int role_id = 0;
    std::string created_at;
    std::string updated_at;
};

struct Role {
    int id = 0;
    std::string name;
    std::string description;
    std::string created_at;
};

struct Permission {
    int id = 0;
    std::string code;
    std::string description;
    std::string created_at;
};

struct Device {
    int id = 0;
    std::string name;
    std::string type;
    std::string ip_address;
    int port = 8080;
    std::string status = "offline";
    std::string model;
    std::string firmware_version;
    std::string created_at;
    std::string updated_at;
};

struct Task {
    int id = 0;
    std::string name;
    std::string description;
    int device_id = 0;
    std::string graph_config;
    std::string status = "stopped";
    std::string schedule_cron;
    std::string created_at;
    std::string updated_at;
};

struct Algorithm {
    int id = 0;
    std::string name;
    std::string version;
    std::string type;
    std::string model_path;
    std::string config_schema;
    std::string plugin_path;
    std::string status = "inactive";
    std::string created_at;
};

struct TaskAlgorithm {
    int id = 0;
    int task_id = 0;
    int algorithm_id = 0;
    std::string params;
    int order_index = 0;
};

struct AlarmRule {
    int id = 0;
    std::string name;
    std::string condition_expr;
    int debounce_seconds = 0;
    std::string notification_channels;
    bool enabled = true;
    std::string created_at;
};

struct AlarmEvent {
    int id = 0;
    int rule_id = 0;
    int task_id = 0;
    std::string evidence_path;
    std::string context;
    std::string triggered_at;
};

struct MonitoringMetric {
    int id = 0;
    std::string metric_type;
    double value = 0.0;
    std::string recorded_at;
};

struct ConfigVersion {
    int id = 0;
    std::string config_key;
    std::string config_value;
    int version = 0;
    std::string created_at;
};

struct Plugin {
    int id = 0;
    std::string name;
    std::string version;
    std::string path;
    std::string signature;
    std::string status = "inactive";
    std::string created_at;
};

struct UpgradeRecord {
    int id = 0;
    std::string target_version;
    std::string type;
    std::string status = "pending";
    std::string rollback_version;
    std::string started_at;
    std::string completed_at;
};

} // namespace models
} // namespace web
} // namespace sophon
