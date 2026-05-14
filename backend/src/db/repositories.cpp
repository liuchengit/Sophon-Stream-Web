#include "db/repositories.h"
#include "db/database_manager.h"
#include <sqlite3.h>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <cstring>

static std::string currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

static std::string safeStr(const char* s) {
    return s ? std::string(s) : "";
}

static int getIntColumn(char** colName, char** argv, int argc, const char* name, int defaultVal = 0) {
    for (int i = 0; i < argc; i++) {
        if (std::strcmp(colName[i], name) == 0 && argv[i]) {
            return std::stoi(argv[i]);
        }
    }
    return defaultVal;
}

static std::string getStrColumn(char** colName, char** argv, int argc, const char* name) {
    for (int i = 0; i < argc; i++) {
        if (std::strcmp(colName[i], name) == 0) {
            return safeStr(argv[i]);
        }
    }
    return "";
}

static double getDoubleColumn(char** colName, char** argv, int argc, const char* name, double defaultVal = 0.0) {
    for (int i = 0; i < argc; i++) {
        if (std::strcmp(colName[i], name) == 0 && argv[i]) {
            return std::stod(argv[i]);
        }
    }
    return defaultVal;
}

namespace sophon {
namespace web {
namespace db {

// UserRepository implementation

static int userCallback(void* data, int argc, char** argv, char** colName) {
    auto* users = static_cast<std::vector<models::User>*>(data);
    models::User u;
    u.id = getIntColumn(colName, argv, argc, "id");
    u.username = getStrColumn(colName, argv, argc, "username");
    u.password_hash = getStrColumn(colName, argv, argc, "password_hash");
    u.email = getStrColumn(colName, argv, argc, "email");
    u.role_id = getIntColumn(colName, argv, argc, "role_id");
    u.created_at = getStrColumn(colName, argv, argc, "created_at");
    u.updated_at = getStrColumn(colName, argv, argc, "updated_at");
    users->push_back(u);
    return 0;
}

std::optional<models::User> UserRepository::findByUsername(const std::string& username) {
    std::vector<models::User> users;
    std::string sql = "SELECT id, username, password_hash, email, role_id, created_at, updated_at FROM users WHERE username='" + username + "'";
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), userCallback, &users, &errMsg);
    return users.empty() ? std::nullopt : std::make_optional(users[0]);
}

std::optional<models::User> UserRepository::findById(int id) {
    std::vector<models::User> users;
    std::string sql = "SELECT id, username, password_hash, email, role_id, created_at, updated_at FROM users WHERE id=" + std::to_string(id);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), userCallback, &users, &errMsg);
    return users.empty() ? std::nullopt : std::make_optional(users[0]);
}

std::vector<models::User> UserRepository::findAll(int page, int limit) {
    std::vector<models::User> users;
    int offset = (page - 1) * limit;
    std::string sql = "SELECT id, username, email, role_id, created_at, updated_at FROM users LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), userCallback, &users, &errMsg);
    return users;
}

int UserRepository::create(const models::User& user) {
    std::string now = currentTimestamp();
    std::string sql = "INSERT INTO users (username, password_hash, email, role_id, created_at, updated_at) VALUES ('" +
                      user.username + "', '" + user.password_hash + "', '" + user.email + "', " +
                      std::to_string(user.role_id) + ", '" + now + "', '" + now + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
    return 0;
}

bool UserRepository::update(int id, const models::User& user) {
    std::string now = currentTimestamp();
    std::string sql = "UPDATE users SET username='" + user.username + "', email='" + user.email +
                      "', role_id=" + std::to_string(user.role_id) + ", updated_at='" + now + "' WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

bool UserRepository::remove(int id) {
    std::string sql = "DELETE FROM users WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

// DeviceRepository implementation

static int deviceCallback(void* data, int argc, char** argv, char** colName) {
    auto* devices = static_cast<std::vector<models::Device>*>(data);
    models::Device d;
    d.id = getIntColumn(colName, argv, argc, "id");
    d.name = getStrColumn(colName, argv, argc, "name");
    d.type = getStrColumn(colName, argv, argc, "type");
    d.ip_address = getStrColumn(colName, argv, argc, "ip_address");
    d.port = getIntColumn(colName, argv, argc, "port");
    d.status = getStrColumn(colName, argv, argc, "status");
    d.model = getStrColumn(colName, argv, argc, "model");
    d.firmware_version = getStrColumn(colName, argv, argc, "firmware_version");
    d.created_at = getStrColumn(colName, argv, argc, "created_at");
    d.updated_at = getStrColumn(colName, argv, argc, "updated_at");
    devices->push_back(d);
    return 0;
}

std::optional<models::Device> DeviceRepository::findById(int id) {
    std::vector<models::Device> devices;
    std::string sql = "SELECT * FROM devices WHERE id=" + std::to_string(id);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), deviceCallback, &devices, &errMsg);
    return devices.empty() ? std::nullopt : std::make_optional(devices[0]);
}

std::vector<models::Device> DeviceRepository::findAll(const std::string& status, const std::string& type, int page, int limit) {
    std::vector<models::Device> devices;
    std::string sql = "SELECT * FROM devices WHERE 1=1";
    if (!status.empty()) sql += " AND status='" + status + "'";
    if (!type.empty()) sql += " AND type='" + type + "'";
    int offset = (page - 1) * limit;
    sql += " ORDER BY created_at DESC LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), deviceCallback, &devices, &errMsg);
    return devices;
}

int DeviceRepository::count(const std::string& status, const std::string& type) {
    std::string sql = "SELECT COUNT(*) FROM devices WHERE 1=1";
    if (!status.empty()) sql += " AND status='" + status + "'";
    if (!type.empty()) sql += " AND type='" + type + "'";
    
    int count = 0;
    auto countCb = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0 && argv[0]) *static_cast<int*>(data) = std::stoi(argv[0]);
        return 0;
    };
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), countCb, &count, &errMsg);
    return count;
}

int DeviceRepository::create(const models::Device& device) {
    std::string now = currentTimestamp();
    std::string sql = "INSERT INTO devices (name, type, ip_address, port, status, model, firmware_version, created_at, updated_at) VALUES ('" +
                      device.name + "', '" + device.type + "', '" + device.ip_address + "', " +
                      std::to_string(device.port) + ", '" + device.status + "', '" + device.model + "', '" +
                      device.firmware_version + "', '" + now + "', '" + now + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
    return 0;
}

bool DeviceRepository::update(int id, const models::Device& device) {
    std::string now = currentTimestamp();
    std::string sql = "UPDATE devices SET name='" + device.name + "', type='" + device.type +
                      "', ip_address='" + device.ip_address + "', port=" + std::to_string(device.port) +
                      ", status='" + device.status + "', model='" + device.model +
                      "', firmware_version='" + device.firmware_version + "', updated_at='" + now + "' WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

bool DeviceRepository::remove(int id) {
    return DatabaseManager::instance().execute("DELETE FROM devices WHERE id=" + std::to_string(id));
}

bool DeviceRepository::updateStatus(int id, const std::string& status) {
    return DatabaseManager::instance().execute("UPDATE devices SET status='" + status + "' WHERE id=" + std::to_string(id));
}

// TaskRepository implementation

static int taskCallback(void* data, int argc, char** argv, char** colName) {
    auto* tasks = static_cast<std::vector<models::Task>*>(data);
    models::Task t;
    t.id = getIntColumn(colName, argv, argc, "id");
    t.name = getStrColumn(colName, argv, argc, "name");
    t.description = getStrColumn(colName, argv, argc, "description");
    t.device_id = getIntColumn(colName, argv, argc, "device_id");
    t.graph_config = getStrColumn(colName, argv, argc, "graph_config");
    t.status = getStrColumn(colName, argv, argc, "status");
    t.schedule_cron = getStrColumn(colName, argv, argc, "schedule_cron");
    t.created_at = getStrColumn(colName, argv, argc, "created_at");
    t.updated_at = getStrColumn(colName, argv, argc, "updated_at");
    tasks->push_back(t);
    return 0;
}

std::optional<models::Task> TaskRepository::findById(int id) {
    std::vector<models::Task> tasks;
    std::string sql = "SELECT * FROM tasks WHERE id=" + std::to_string(id);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), taskCallback, &tasks, &errMsg);
    return tasks.empty() ? std::nullopt : std::make_optional(tasks[0]);
}

std::vector<models::Task> TaskRepository::findAll(const std::string& status, int page, int limit) {
    std::vector<models::Task> tasks;
    std::string sql = "SELECT * FROM tasks WHERE 1=1";
    if (!status.empty()) sql += " AND status='" + status + "'";
    int offset = (page - 1) * limit;
    sql += " ORDER BY created_at DESC LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), taskCallback, &tasks, &errMsg);
    return tasks;
}

int TaskRepository::count(const std::string& status) {
    std::string sql = "SELECT COUNT(*) FROM tasks WHERE 1=1";
    if (!status.empty()) sql += " AND status='" + status + "'";
    int count = 0;
    auto countCb = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0 && argv[0]) *static_cast<int*>(data) = std::stoi(argv[0]);
        return 0;
    };
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), countCb, &count, &errMsg);
    return count;
}

int TaskRepository::create(const models::Task& task) {
    std::string now = currentTimestamp();
    std::string sql = "INSERT INTO tasks (name, description, device_id, graph_config, status, schedule_cron, created_at, updated_at) VALUES ('" +
                      task.name + "', '" + task.description + "', " + std::to_string(task.device_id) +
                      ", '" + task.graph_config + "', '" + task.status + "', '" + task.schedule_cron +
                      "', '" + now + "', '" + now + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
    return 0;
}

bool TaskRepository::update(int id, const models::Task& task) {
    std::string now = currentTimestamp();
    std::string sql = "UPDATE tasks SET name='" + task.name + "', description='" + task.description +
                      "', device_id=" + std::to_string(task.device_id) +
                      ", graph_config='" + task.graph_config + "', status='" + task.status +
                      "', schedule_cron='" + task.schedule_cron + "', updated_at='" + now + "' WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

bool TaskRepository::remove(int id) {
    return DatabaseManager::instance().execute("DELETE FROM tasks WHERE id=" + std::to_string(id));
}

bool TaskRepository::updateStatus(int id, const std::string& status) {
    return DatabaseManager::instance().execute("UPDATE tasks SET status='" + status + "' WHERE id=" + std::to_string(id));
}

// AlgorithmRepository implementation

static int algoCallback(void* data, int argc, char** argv, char** colName) {
    auto* algos = static_cast<std::vector<models::Algorithm>*>(data);
    models::Algorithm a;
    a.id = getIntColumn(colName, argv, argc, "id");
    a.name = getStrColumn(colName, argv, argc, "name");
    a.version = getStrColumn(colName, argv, argc, "version");
    a.type = getStrColumn(colName, argv, argc, "type");
    a.model_path = getStrColumn(colName, argv, argc, "model_path");
    a.config_schema = getStrColumn(colName, argv, argc, "config_schema");
    a.plugin_path = getStrColumn(colName, argv, argc, "plugin_path");
    a.status = getStrColumn(colName, argv, argc, "status");
    a.created_at = getStrColumn(colName, argv, argc, "created_at");
    algos->push_back(a);
    return 0;
}

std::optional<models::Algorithm> AlgorithmRepository::findById(int id) {
    std::vector<models::Algorithm> algos;
    std::string sql = "SELECT * FROM algorithms WHERE id=" + std::to_string(id);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), algoCallback, &algos, &errMsg);
    return algos.empty() ? std::nullopt : std::make_optional(algos[0]);
}

std::vector<models::Algorithm> AlgorithmRepository::findAll(int page, int limit) {
    std::vector<models::Algorithm> algos;
    int offset = (page - 1) * limit;
    std::string sql = "SELECT * FROM algorithms ORDER BY created_at DESC LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), algoCallback, &algos, &errMsg);
    return algos;
}

int AlgorithmRepository::count() {
    int count = 0;
    auto countCb = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0 && argv[0]) *static_cast<int*>(data) = std::stoi(argv[0]);
        return 0;
    };
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), "SELECT COUNT(*) FROM algorithms", countCb, &count, &errMsg);
    return count;
}

int AlgorithmRepository::create(const models::Algorithm& algo) {
    std::string sql = "INSERT INTO algorithms (name, version, type, model_path, config_schema, plugin_path, status) VALUES ('" +
                      algo.name + "', '" + algo.version + "', '" + algo.type + "', '" +
                      algo.model_path + "', '" + algo.config_schema + "', '" + algo.plugin_path + "', '" + algo.status + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
    return 0;
}

bool AlgorithmRepository::update(int id, const models::Algorithm& algo) {
    std::string sql = "UPDATE algorithms SET name='" + algo.name + "', version='" + algo.version +
                      "', type='" + algo.type + "', model_path='" + algo.model_path +
                      "', config_schema='" + algo.config_schema + "', plugin_path='" + algo.plugin_path +
                      "', status='" + algo.status + "' WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

bool AlgorithmRepository::remove(int id) {
    return DatabaseManager::instance().execute("DELETE FROM algorithms WHERE id=" + std::to_string(id));
}

// AlarmRuleRepository implementation

static int alarmRuleCallback(void* data, int argc, char** argv, char** colName) {
    auto* rules = static_cast<std::vector<models::AlarmRule>*>(data);
    models::AlarmRule r;
    r.id = getIntColumn(colName, argv, argc, "id");
    r.name = getStrColumn(colName, argv, argc, "name");
    r.condition_expr = getStrColumn(colName, argv, argc, "condition_expr");
    r.debounce_seconds = getIntColumn(colName, argv, argc, "debounce_seconds");
    r.notification_channels = getStrColumn(colName, argv, argc, "notification_channels");
    r.enabled = getIntColumn(colName, argv, argc, "enabled") != 0;
    r.created_at = getStrColumn(colName, argv, argc, "created_at");
    rules->push_back(r);
    return 0;
}

std::optional<models::AlarmRule> AlarmRuleRepository::findById(int id) {
    std::vector<models::AlarmRule> rules;
    std::string sql = "SELECT * FROM alarm_rules WHERE id=" + std::to_string(id);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), alarmRuleCallback, &rules, &errMsg);
    return rules.empty() ? std::nullopt : std::make_optional(rules[0]);
}

std::vector<models::AlarmRule> AlarmRuleRepository::findAll(bool enabledOnly) {
    std::vector<models::AlarmRule> rules;
    std::string sql = "SELECT * FROM alarm_rules";
    if (enabledOnly) sql += " WHERE enabled=1";
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), alarmRuleCallback, &rules, &errMsg);
    return rules;
}

int AlarmRuleRepository::create(const models::AlarmRule& rule) {
    std::string sql = "INSERT INTO alarm_rules (name, condition_expr, debounce_seconds, notification_channels, enabled) VALUES ('" +
                      rule.name + "', '" + rule.condition_expr + "', " + std::to_string(rule.debounce_seconds) +
                      ", '" + rule.notification_channels + "', " + (rule.enabled ? "1" : "0") + ")";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
    return 0;
}

bool AlarmRuleRepository::update(int id, const models::AlarmRule& rule) {
    std::string sql = "UPDATE alarm_rules SET name='" + rule.name + "', condition_expr='" + rule.condition_expr +
                      "', debounce_seconds=" + std::to_string(rule.debounce_seconds) +
                      ", notification_channels='" + rule.notification_channels +
                      "', enabled=" + (rule.enabled ? "1" : "0") + " WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

bool AlarmRuleRepository::remove(int id) {
    return DatabaseManager::instance().execute("DELETE FROM alarm_rules WHERE id=" + std::to_string(id));
}

bool AlarmRuleRepository::toggle(int id, bool enabled) {
    return DatabaseManager::instance().execute("UPDATE alarm_rules SET enabled=" + std::to_string(enabled ? 1 : 0) + " WHERE id=" + std::to_string(id));
}

// AlarmEventRepository implementation

static int alarmEventCallback(void* data, int argc, char** argv, char** colName) {
    auto* events = static_cast<std::vector<models::AlarmEvent>*>(data);
    models::AlarmEvent e;
    e.id = getIntColumn(colName, argv, argc, "id");
    e.rule_id = getIntColumn(colName, argv, argc, "rule_id");
    e.task_id = getIntColumn(colName, argv, argc, "task_id");
    e.evidence_path = getStrColumn(colName, argv, argc, "evidence_path");
    e.context = getStrColumn(colName, argv, argc, "context");
    e.triggered_at = getStrColumn(colName, argv, argc, "triggered_at");
    events->push_back(e);
    return 0;
}

std::optional<models::AlarmEvent> AlarmEventRepository::findById(int id) {
    std::vector<models::AlarmEvent> events;
    std::string sql = "SELECT * FROM alarm_events WHERE id=" + std::to_string(id);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), alarmEventCallback, &events, &errMsg);
    return events.empty() ? std::nullopt : std::make_optional(events[0]);
}

std::vector<models::AlarmEvent> AlarmEventRepository::findAll(int page, int limit) {
    std::vector<models::AlarmEvent> events;
    int offset = (page - 1) * limit;
    std::string sql = "SELECT * FROM alarm_events ORDER BY triggered_at DESC LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), alarmEventCallback, &events, &errMsg);
    return events;
}

int AlarmEventRepository::count() {
    int count = 0;
    auto countCb = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0 && argv[0]) *static_cast<int*>(data) = std::stoi(argv[0]);
        return 0;
    };
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), "SELECT COUNT(*) FROM alarm_events", countCb, &count, &errMsg);
    return count;
}

int AlarmEventRepository::create(const models::AlarmEvent& event) {
    std::string now = currentTimestamp();
    std::string sql = "INSERT INTO alarm_events (rule_id, task_id, evidence_path, context, triggered_at) VALUES (" +
                      std::to_string(event.rule_id) + ", " + std::to_string(event.task_id) +
                      ", '" + event.evidence_path + "', '" + event.context + "', '" + now + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
    return 0;
}

// MonitoringMetricRepository implementation

static int metricCallback(void* data, int argc, char** argv, char** colName) {
    auto* metrics = static_cast<std::vector<models::MonitoringMetric>*>(data);
    models::MonitoringMetric m;
    m.id = getIntColumn(colName, argv, argc, "id");
    m.metric_type = getStrColumn(colName, argv, argc, "metric_type");
    m.value = getDoubleColumn(colName, argv, argc, "value");
    m.recorded_at = getStrColumn(colName, argv, argc, "recorded_at");
    metrics->push_back(m);
    return 0;
}

int MonitoringMetricRepository::create(const models::MonitoringMetric& metric) {
    std::string now = currentTimestamp();
    std::string sql = "INSERT INTO monitoring_metrics (metric_type, value, recorded_at) VALUES ('" +
                      metric.metric_type + "', " + std::to_string(metric.value) + ", '" + now + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
    return 0;
}

std::vector<models::MonitoringMetric> MonitoringMetricRepository::findByType(const std::string& type, const std::string& startTime, const std::string& endTime) {
    std::vector<models::MonitoringMetric> metrics;
    std::string sql = "SELECT * FROM monitoring_metrics WHERE metric_type='" + type + "'";
    if (!startTime.empty()) sql += " AND recorded_at >= '" + startTime + "'";
    if (!endTime.empty()) sql += " AND recorded_at <= '" + endTime + "'";
    sql += " ORDER BY recorded_at DESC LIMIT 100";
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), metricCallback, &metrics, &errMsg);
    return metrics;
}

bool MonitoringMetricRepository::cleanOldMetrics(int daysToKeep) {
    std::string sql = "DELETE FROM monitoring_metrics WHERE recorded_at < datetime('now', '-" + std::to_string(daysToKeep) + " days')";
    return DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

// ConfigVersionRepository implementation

static int configCallback(void* data, int argc, char** argv, char** colName) {
    auto* configs = static_cast<std::vector<models::ConfigVersion>*>(data);
    models::ConfigVersion c;
    c.id = getIntColumn(colName, argv, argc, "id");
    c.config_key = getStrColumn(colName, argv, argc, "config_key");
    c.config_value = getStrColumn(colName, argv, argc, "config_value");
    c.version = getIntColumn(colName, argv, argc, "version");
    c.created_at = getStrColumn(colName, argv, argc, "created_at");
    configs->push_back(c);
    return 0;
}

std::optional<models::ConfigVersion> ConfigVersionRepository::findLatest(const std::string& key) {
    std::vector<models::ConfigVersion> configs;
    std::string sql = "SELECT * FROM config_versions WHERE config_key='" + key + "' ORDER BY version DESC LIMIT 1";
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), configCallback, &configs, &errMsg);
    return configs.empty() ? std::nullopt : std::make_optional(configs[0]);
}

std::vector<models::ConfigVersion> ConfigVersionRepository::findHistory(const std::string& key, int limit) {
    std::vector<models::ConfigVersion> configs;
    std::string sql = "SELECT * FROM config_versions WHERE config_key='" + key + "' ORDER BY version DESC LIMIT " + std::to_string(limit);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), configCallback, &configs, &errMsg);
    return configs;
}

int ConfigVersionRepository::create(const std::string& key, const std::string& value) {
    std::string sql = "INSERT INTO config_versions (config_key, config_value, version) VALUES ('" +
                      key + "', '" + value + "', (SELECT COALESCE(MAX(version), 0) + 1 FROM config_versions WHERE config_key='" + key + "'))";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
    return 0;
}

bool ConfigVersionRepository::rollback(const std::string& key, int version) {
    auto opt = findLatest(key);
    if (!opt) return false;
    
    std::string sql = "SELECT config_value FROM config_versions WHERE config_key='" + key + "' AND version=" + std::to_string(version);
    std::string value;
    auto valueCb = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0 && argv[0]) *static_cast<std::string*>(data) = argv[0];
        return 0;
    };
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), valueCb, &value, &errMsg);
    
    if (value.empty()) return false;
    return create(key, value);
}

// PluginRepository implementation

static int pluginCallback(void* data, int argc, char** argv, char** colName) {
    auto* plugins = static_cast<std::vector<models::Plugin>*>(data);
    models::Plugin p;
    p.id = getIntColumn(colName, argv, argc, "id");
    p.name = getStrColumn(colName, argv, argc, "name");
    p.version = getStrColumn(colName, argv, argc, "version");
    p.path = getStrColumn(colName, argv, argc, "path");
    p.signature = getStrColumn(colName, argv, argc, "signature");
    p.status = getStrColumn(colName, argv, argc, "status");
    p.created_at = getStrColumn(colName, argv, argc, "created_at");
    plugins->push_back(p);
    return 0;
}

std::optional<models::Plugin> PluginRepository::findById(int id) {
    std::vector<models::Plugin> plugins;
    std::string sql = "SELECT * FROM plugins WHERE id=" + std::to_string(id);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), pluginCallback, &plugins, &errMsg);
    return plugins.empty() ? std::nullopt : std::make_optional(plugins[0]);
}

std::optional<models::Plugin> PluginRepository::findByName(const std::string& name) {
    std::vector<models::Plugin> plugins;
    std::string sql = "SELECT * FROM plugins WHERE name='" + name + "'";
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), pluginCallback, &plugins, &errMsg);
    return plugins.empty() ? std::nullopt : std::make_optional(plugins[0]);
}

std::vector<models::Plugin> PluginRepository::findAll(const std::string& status) {
    std::vector<models::Plugin> plugins;
    std::string sql = "SELECT * FROM plugins";
    if (!status.empty()) sql += " WHERE status='" + status + "'";
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), pluginCallback, &plugins, &errMsg);
    return plugins;
}

int PluginRepository::create(const models::Plugin& plugin) {
    std::string sql = "INSERT INTO plugins (name, version, path, signature, status) VALUES ('" +
                      plugin.name + "', '" + plugin.version + "', '" + plugin.path + "', '" +
                      plugin.signature + "', '" + plugin.status + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
    return 0;
}

bool PluginRepository::update(int id, const models::Plugin& plugin) {
    std::string sql = "UPDATE plugins SET name='" + plugin.name + "', version='" + plugin.version +
                      "', path='" + plugin.path + "', signature='" + plugin.signature +
                      "', status='" + plugin.status + "' WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

bool PluginRepository::remove(int id) {
    return DatabaseManager::instance().execute("DELETE FROM plugins WHERE id=" + std::to_string(id));
}

// UpgradeRecordRepository implementation

static int upgradeCallback(void* data, int argc, char** argv, char** colName) {
    auto* records = static_cast<std::vector<models::UpgradeRecord>*>(data);
    models::UpgradeRecord r;
    r.id = getIntColumn(colName, argv, argc, "id");
    r.target_version = getStrColumn(colName, argv, argc, "target_version");
    r.type = getStrColumn(colName, argv, argc, "type");
    r.status = getStrColumn(colName, argv, argc, "status");
    r.rollback_version = getStrColumn(colName, argv, argc, "rollback_version");
    r.started_at = getStrColumn(colName, argv, argc, "started_at");
    r.completed_at = getStrColumn(colName, argv, argc, "completed_at");
    records->push_back(r);
    return 0;
}

std::optional<models::UpgradeRecord> UpgradeRecordRepository::findById(int id) {
    std::vector<models::UpgradeRecord> records;
    std::string sql = "SELECT * FROM upgrade_records WHERE id=" + std::to_string(id);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), upgradeCallback, &records, &errMsg);
    return records.empty() ? std::nullopt : std::make_optional(records[0]);
}

std::vector<models::UpgradeRecord> UpgradeRecordRepository::findAll(int page, int limit) {
    std::vector<models::UpgradeRecord> records;
    int offset = (page - 1) * limit;
    std::string sql = "SELECT * FROM upgrade_records ORDER BY started_at DESC LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), upgradeCallback, &records, &errMsg);
    return records;
}

int UpgradeRecordRepository::create(const models::UpgradeRecord& record) {
    std::string now = currentTimestamp();
    std::string sql = "INSERT INTO upgrade_records (target_version, type, status, rollback_version, started_at) VALUES ('" +
                      record.target_version + "', '" + record.type + "', '" + record.status + "', '" +
                      record.rollback_version + "', '" + now + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
    return 0;
}

bool UpgradeRecordRepository::update(int id, const models::UpgradeRecord& record) {
    std::string now = currentTimestamp();
    std::string sql = "UPDATE upgrade_records SET target_version='" + record.target_version +
                      "', type='" + record.type + "', status='" + record.status +
                      "', rollback_version='" + record.rollback_version +
                      "', completed_at='" + now + "' WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

// WorkflowRepository implementation

static int workflowCallback(void* data, int argc, char** argv, char** colName) {
    auto* workflows = static_cast<std::vector<models::Workflow>*>(data);
    models::Workflow w;
    w.id = getIntColumn(colName, argv, argc, "id");
    w.name = getStrColumn(colName, argv, argc, "name");
    w.description = getStrColumn(colName, argv, argc, "description");
    w.status = getStrColumn(colName, argv, argc, "status");
    w.created_at = getStrColumn(colName, argv, argc, "created_at");
    w.updated_at = getStrColumn(colName, argv, argc, "updated_at");
    workflows->push_back(w);
    return 0;
}

std::optional<models::Workflow> WorkflowRepository::findById(int id) {
    std::vector<models::Workflow> workflows;
    std::string sql = "SELECT * FROM workflows WHERE id=" + std::to_string(id);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), workflowCallback, &workflows, &errMsg);
    return workflows.empty() ? std::nullopt : std::make_optional(workflows[0]);
}

std::vector<models::Workflow> WorkflowRepository::findAll(const std::string& status, int page, int limit) {
    std::vector<models::Workflow> workflows;
    std::string sql = "SELECT * FROM workflows";
    if (!status.empty()) sql += " WHERE status='" + status + "'";
    int offset = (page - 1) * limit;
    sql += " ORDER BY updated_at DESC LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), workflowCallback, &workflows, &errMsg);
    return workflows;
}

int WorkflowRepository::count(const std::string& status) {
    std::string sql = "SELECT COUNT(*) FROM workflows";
    if (!status.empty()) sql += " WHERE status='" + status + "'";
    int count = 0;
    auto cb = [](void* data, int argc, char** argv, char**) -> int {
        if (argc > 0 && argv[0]) *static_cast<int*>(data) = std::atoi(argv[0]);
        return 0;
    };
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), cb, &count, &errMsg);
    return count;
}

int WorkflowRepository::create(const models::Workflow& workflow) {
    std::string now = currentTimestamp();
    std::string sql = "INSERT INTO workflows (name, description, status, updated_at) VALUES ('" +
                      workflow.name + "', '" + workflow.description + "', '" + workflow.status + "', '" + now + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

bool WorkflowRepository::update(int id, const models::Workflow& workflow) {
    std::string now = currentTimestamp();
    std::string sql = "UPDATE workflows SET name='" + workflow.name +
                      "', description='" + workflow.description +
                      "', status='" + workflow.status +
                      "', updated_at='" + now + "' WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
}

bool WorkflowRepository::remove(int id) {
    std::string sql = "DELETE FROM workflows WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
}

bool WorkflowRepository::updateStatus(int id, const std::string& status) {
    std::string now = currentTimestamp();
    std::string sql = "UPDATE workflows SET status='" + status + "', updated_at='" + now + "' WHERE id=" + std::to_string(id);
    return DatabaseManager::instance().execute(sql);
}

// WorkflowNodeRepository implementation

static int workflowNodeCallback(void* data, int argc, char** argv, char** colName) {
    auto* nodes = static_cast<std::vector<models::WorkflowNode>*>(data);
    models::WorkflowNode n;
    n.id = getIntColumn(colName, argv, argc, "id");
    n.workflow_id = getIntColumn(colName, argv, argc, "workflow_id");
    n.node_id = getStrColumn(colName, argv, argc, "node_id");
    n.node_type = getStrColumn(colName, argv, argc, "node_type");
    n.position_x = getDoubleColumn(colName, argv, argc, "position_x");
    n.position_y = getDoubleColumn(colName, argv, argc, "position_y");
    n.config_json = getStrColumn(colName, argv, argc, "config_json");
    n.label = getStrColumn(colName, argv, argc, "label");
    nodes->push_back(n);
    return 0;
}

std::vector<models::WorkflowNode> WorkflowNodeRepository::findByWorkflowId(int workflowId) {
    std::vector<models::WorkflowNode> nodes;
    std::string sql = "SELECT * FROM workflow_nodes WHERE workflow_id=" + std::to_string(workflowId);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), workflowNodeCallback, &nodes, &errMsg);
    return nodes;
}

int WorkflowNodeRepository::create(const models::WorkflowNode& node) {
    std::string sql = "INSERT INTO workflow_nodes (workflow_id, node_id, node_type, position_x, position_y, config_json, label) VALUES (" +
                      std::to_string(node.workflow_id) + ", '" + node.node_id + "', '" + node.node_type + "', " +
                      std::to_string(node.position_x) + ", " + std::to_string(node.position_y) + ", '" +
                      node.config_json + "', '" + node.label + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

bool WorkflowNodeRepository::update(int workflowId, const std::string& nodeId, const models::WorkflowNode& node) {
    std::string sql = "UPDATE workflow_nodes SET position_x=" + std::to_string(node.position_x) +
                      ", position_y=" + std::to_string(node.position_y) +
                      ", config_json='" + node.config_json +
                      "', label='" + node.label +
                      "' WHERE workflow_id=" + std::to_string(workflowId) + " AND node_id='" + nodeId + "'";
    return DatabaseManager::instance().execute(sql);
}

bool WorkflowNodeRepository::removeByWorkflowId(int workflowId) {
    std::string sql = "DELETE FROM workflow_nodes WHERE workflow_id=" + std::to_string(workflowId);
    return DatabaseManager::instance().execute(sql);
}

// WorkflowEdgeRepository implementation

static int workflowEdgeCallback(void* data, int argc, char** argv, char** colName) {
    auto* edges = static_cast<std::vector<models::WorkflowEdge>*>(data);
    models::WorkflowEdge e;
    e.id = getIntColumn(colName, argv, argc, "id");
    e.workflow_id = getIntColumn(colName, argv, argc, "workflow_id");
    e.edge_id = getStrColumn(colName, argv, argc, "edge_id");
    e.source_node = getStrColumn(colName, argv, argc, "source_node");
    e.target_node = getStrColumn(colName, argv, argc, "target_node");
    e.source_handle = getStrColumn(colName, argv, argc, "source_handle");
    e.target_handle = getStrColumn(colName, argv, argc, "target_handle");
    edges->push_back(e);
    return 0;
}

std::vector<models::WorkflowEdge> WorkflowEdgeRepository::findByWorkflowId(int workflowId) {
    std::vector<models::WorkflowEdge> edges;
    std::string sql = "SELECT * FROM workflow_edges WHERE workflow_id=" + std::to_string(workflowId);
    char* errMsg = nullptr;
    sqlite3_exec(reinterpret_cast<sqlite3*>(DatabaseManager::instance().db()), sql.c_str(), workflowEdgeCallback, &edges, &errMsg);
    return edges;
}

int WorkflowEdgeRepository::create(const models::WorkflowEdge& edge) {
    std::string sql = "INSERT INTO workflow_edges (workflow_id, edge_id, source_node, target_node, source_handle, target_handle) VALUES (" +
                      std::to_string(edge.workflow_id) + ", '" + edge.edge_id + "', '" + edge.source_node + "', '" +
                      edge.target_node + "', '" + edge.source_handle + "', '" + edge.target_handle + "')";
    DatabaseManager::instance().execute(sql);
    return DatabaseManager::instance().lastInsertRowId();
}

bool WorkflowEdgeRepository::removeByWorkflowId(int workflowId) {
    std::string sql = "DELETE FROM workflow_edges WHERE workflow_id=" + std::to_string(workflowId);
    return DatabaseManager::instance().execute(sql);
}

} // namespace db
} // namespace web
} // namespace sophon
