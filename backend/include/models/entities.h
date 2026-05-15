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
    // GB28181 fields
    std::string gb_alarm_type;
    int alarm_method = 5;
    std::string subscribe_status;
    std::string subscribe_expires;
    int device_id = 0;
    int channel_id = 0;
    std::string alarm_priority;
    std::string alarm_description;
};

struct AlarmSubscription {
    int id = 0;
    int device_id = 0;
    int channel_id = 0;
    std::string alarm_types;
    std::string subscribe_status;
    std::string subscribe_time;
    std::string expires_time;
    std::string last_heartbeat;
    std::string created_at;
    std::string updated_at;
};

struct AlarmEvent {
    int id = 0;
    int rule_id = 0;
    int task_id = 0;
    std::string evidence_path;
    std::string context;
    std::string triggered_at;
    // GB28181 fields
    std::string gb_alarm_code;
    std::string alarm_priority;
    std::string alarm_type;
    int device_id = 0;
    int channel_id = 0;
    std::string sip_transaction_id;
    std::string alarm_description;
    std::string handled_status;
    std::string handled_at;
    int handled_by = 0;
    std::string handle_result;
    int alarm_method = 5;
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

struct Workflow {
    int id = 0;
    std::string name;
    std::string description;
    std::string status = "draft";
    std::string created_at;
    std::string updated_at;
};

struct WorkflowNode {
    int id = 0;
    int workflow_id = 0;
    std::string node_id;
    std::string node_type;
    double position_x = 0;
    double position_y = 0;
    std::string config_json = "{}";
    std::string label;
};

struct WorkflowEdge {
    int id = 0;
    int workflow_id = 0;
    std::string edge_id;
    std::string source_node;
    std::string target_node;
    std::string source_handle = "default";
    std::string target_handle = "default";
};

struct WorkflowExecution {
    int id = 0;
    int workflow_id = 0;
    std::string status = "running";
    std::string started_at;
    std::string finished_at;
    std::string error_message;
};

struct WorkflowExecutionNode {
    int id = 0;
    int execution_id = 0;
    std::string node_id;
    std::string node_type;
    std::string label;
    std::string status = "pending";
    std::string started_at;
    std::string finished_at;
    std::string error_message;
};

} // namespace models
} // namespace web
} // namespace sophon
