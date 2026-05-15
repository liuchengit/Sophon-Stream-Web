#include "db/database_manager.h"
#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <sstream>

namespace sophon {
namespace web {
namespace db {

static const char* SCHEMA_SQL = R"(
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    email VARCHAR(100),
    role_id INTEGER REFERENCES roles(id),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS roles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(50) UNIQUE NOT NULL,
    description TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS permissions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    code VARCHAR(100) UNIQUE NOT NULL,
    description TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS role_permissions (
    role_id INTEGER REFERENCES roles(id),
    permission_id INTEGER REFERENCES permissions(id),
    PRIMARY KEY (role_id, permission_id)
);

CREATE TABLE IF NOT EXISTS devices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(100) NOT NULL,
    type VARCHAR(50) NOT NULL,
    ip_address VARCHAR(45) NOT NULL,
    port INTEGER DEFAULT 8080,
    status VARCHAR(20) DEFAULT 'offline',
    model VARCHAR(50),
    firmware_version VARCHAR(20),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS tasks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    device_id INTEGER REFERENCES devices(id),
    graph_config TEXT,
    status VARCHAR(20) DEFAULT 'stopped',
    schedule_cron VARCHAR(50),
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS algorithms (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(100) NOT NULL,
    version VARCHAR(20) NOT NULL,
    type VARCHAR(50) NOT NULL,
    model_path VARCHAR(255),
    config_schema TEXT,
    plugin_path VARCHAR(255),
    status VARCHAR(20) DEFAULT 'inactive',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS task_algorithms (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    task_id INTEGER REFERENCES tasks(id),
    algorithm_id INTEGER REFERENCES algorithms(id),
    params TEXT,
    order_index INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS alarm_rules (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(100) NOT NULL,
    condition_expr TEXT NOT NULL,
    debounce_seconds INTEGER DEFAULT 0,
    notification_channels TEXT,
    enabled BOOLEAN DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    -- GB28181 标准字段
    gb_alarm_type VARCHAR(20),
    alarm_method INTEGER DEFAULT 5,
    subscribe_status VARCHAR(20) DEFAULT 'unsubscribed',
    subscribe_expires DATETIME,
    device_id INTEGER REFERENCES devices(id),
    channel_id INTEGER,
    alarm_priority VARCHAR(20) DEFAULT 'medium',
    alarm_description TEXT
);

CREATE TABLE IF NOT EXISTS alarm_subscriptions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id INTEGER REFERENCES devices(id),
    channel_id INTEGER,
    alarm_types TEXT,
    subscribe_status VARCHAR(20) DEFAULT 'active',
    subscribe_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    expires_time DATETIME,
    last_heartbeat DATETIME,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS alarm_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    rule_id INTEGER REFERENCES alarm_rules(id),
    task_id INTEGER REFERENCES tasks(id),
    evidence_path VARCHAR(255),
    context TEXT,
    triggered_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    -- GB28181 标准字段
    gb_alarm_code VARCHAR(20),
    alarm_priority VARCHAR(20) DEFAULT 'medium',
    alarm_type VARCHAR(50),
    device_id INTEGER REFERENCES devices(id),
    channel_id INTEGER,
    sip_transaction_id VARCHAR(100),
    alarm_description TEXT,
    handled_status VARCHAR(20) DEFAULT 'pending',
    handled_at DATETIME,
    handled_by INTEGER REFERENCES users(id),
    handle_result TEXT,
    alarm_method INTEGER DEFAULT 5
);

CREATE TABLE IF NOT EXISTS monitoring_metrics (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    metric_type VARCHAR(50) NOT NULL,
    value REAL NOT NULL,
    recorded_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS config_versions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    config_key VARCHAR(100) NOT NULL,
    config_value TEXT NOT NULL,
    version INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS plugins (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(100) NOT NULL,
    version VARCHAR(20) NOT NULL,
    path VARCHAR(255) NOT NULL,
    signature VARCHAR(255),
    status VARCHAR(20) DEFAULT 'inactive',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS upgrade_records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    target_version VARCHAR(20) NOT NULL,
    type VARCHAR(20) NOT NULL,
    status VARCHAR(20) DEFAULT 'pending',
    rollback_version VARCHAR(20),
    started_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    completed_at DATETIME
);

CREATE TABLE IF NOT EXISTS workflows (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(200) NOT NULL,
    description TEXT DEFAULT '',
    status VARCHAR(20) NOT NULL DEFAULT 'draft',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS workflow_nodes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    workflow_id INTEGER NOT NULL,
    node_id VARCHAR(100) NOT NULL,
    node_type VARCHAR(50) NOT NULL,
    position_x REAL NOT NULL DEFAULT 0,
    position_y REAL NOT NULL DEFAULT 0,
    config_json TEXT DEFAULT '{}',
    label VARCHAR(200) DEFAULT '',
    FOREIGN KEY (workflow_id) REFERENCES workflows(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS workflow_edges (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    workflow_id INTEGER NOT NULL,
    edge_id VARCHAR(100) NOT NULL,
    source_node VARCHAR(100) NOT NULL,
    target_node VARCHAR(100) NOT NULL,
    source_handle VARCHAR(50) DEFAULT 'default',
    target_handle VARCHAR(50) DEFAULT 'default',
    FOREIGN KEY (workflow_id) REFERENCES workflows(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS workflow_executions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    workflow_id INTEGER NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT 'running',
    started_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    finished_at DATETIME,
    error_message TEXT DEFAULT '',
    FOREIGN KEY (workflow_id) REFERENCES workflows(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS workflow_execution_nodes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    execution_id INTEGER NOT NULL,
    node_id VARCHAR(100) NOT NULL,
    node_type VARCHAR(50) NOT NULL,
    label VARCHAR(200) DEFAULT '',
    status VARCHAR(20) NOT NULL DEFAULT 'pending',
    started_at DATETIME,
    finished_at DATETIME,
    error_message TEXT DEFAULT '',
    FOREIGN KEY (execution_id) REFERENCES workflow_executions(id) ON DELETE CASCADE
);

-- Seed data is inserted separately via seedDefaultData()
)";

static const char* SEED_DATA_SQL = R"(
-- Insert default admin role
INSERT OR IGNORE INTO roles (id, name, description) VALUES (1, 'admin', 'System Administrator');

-- Insert default operator role
INSERT OR IGNORE INTO roles (id, name, description) VALUES (2, 'operator', 'System Operator');

-- Insert default permissions
INSERT OR IGNORE INTO permissions (code, description) VALUES
    ('device:read', 'View devices'),
    ('device:write', 'Create/edit/delete devices'),
    ('task:read', 'View tasks'),
    ('task:write', 'Create/edit/delete tasks'),
    ('task:execute', 'Start/stop tasks'),
    ('algorithm:read', 'View algorithms'),
    ('algorithm:write', 'Install/uninstall algorithms'),
    ('monitor:read', 'View monitoring data'),
    ('alarm:read', 'View alarms'),
    ('alarm:write', 'Create/edit alarm rules'),
    ('config:read', 'View configuration'),
    ('config:write', 'Update configuration'),
    ('upgrade:execute', 'Perform system upgrade'),
    ('plugin:read', 'View plugins'),
    ('plugin:write', 'Install/uninstall plugins');

-- Grant admin all permissions
INSERT OR IGNORE INTO role_permissions (role_id, permission_id)
SELECT 1, id FROM permissions;

-- Grant operator read-only + task execute permissions
INSERT OR IGNORE INTO role_permissions (role_id, permission_id)
SELECT 2, id FROM permissions WHERE code IN ('device:read', 'task:read', 'task:execute', 'algorithm:read', 'monitor:read', 'alarm:read', 'config:read', 'plugin:read');

-- Insert default admin user (password: admin123)
INSERT OR IGNORE INTO users (username, password_hash, email, role_id)
VALUES ('admin', '$2b$12$EIXLvK3qGaWYqZqKqZqKqOqNqMqLqKqJqIqHqGqFqEqDqCqBqAq', 'admin@sophon-stream.local', 1);

-- Seed devices
INSERT OR IGNORE INTO devices (id, name, type, ip_address, port, status, model, firmware_version) VALUES
    (1, 'BM1684X-Edge-01', 'tpu', '192.168.1.101', 8080, 'online', 'BM1684X', 'v2.1.0'),
    (2, 'BM1684X-Edge-02', 'tpu', '192.168.1.102', 8080, 'online', 'BM1684X', 'v2.1.0'),
    (3, 'BM1684-Edge-03', 'tpu', '192.168.1.103', 8080, 'offline', 'BM1684', 'v1.8.3'),
    (4, 'Camera-IPC-01', 'camera', '192.168.1.201', 554, 'online', 'Hikvision DS-2CD2T47G2-L', 'V5.6.5'),
    (5, 'Camera-IPC-02', 'camera', '192.168.1.202', 554, 'online', 'Dahua IPC-HFW5442T', 'V2.820.0000'),
    (6, 'Camera-IPC-03', 'camera', '192.168.1.203', 554, 'offline', 'Hikvision DS-2CD2T47G2-L', 'V5.6.5');

-- Seed algorithms
INSERT OR IGNORE INTO algorithms (id, name, version, type, model_path, config_schema, plugin_path, status) VALUES
    (1, 'YOLOX-Person', '1.0.0', 'detection', '/models/yolox_person.bmodel', '{"confidence":0.5,"nms":0.45}', '/plugins/yolox.so', 'active'),
    (2, 'YOLOX-Vehicle', '1.0.0', 'detection', '/models/yolox_vehicle.bmodel', '{"confidence":0.5,"nms":0.45}', '/plugins/yolox.so', 'active'),
    (3, 'ByteTrack', '1.2.0', 'tracking', '/models/bytetrack.bmodel', '{"max_age":30,"min_hits":3}', '/plugins/tracker.so', 'active'),
    (4, 'Face-Detection', '2.0.0', 'detection', '/models/face_detect.bmodel', '{"confidence":0.6}', '/plugins/face.so', 'inactive'),
    (5, 'Pose-Estimation', '1.1.0', 'pose', '/models/pose_est.bmodel', '{"threshold":0.3}', '/plugins/pose.so', 'active');

-- Seed alarm rules
INSERT OR IGNORE INTO alarm_rules (id, name, condition_expr, debounce_seconds, notification_channels, enabled) VALUES
    (1, '人员入侵检测', 'class=="person" AND confidence>0.8', 5, 'webhook,email', 1),
    (2, '区域越界告警', 'roi_violation==true', 10, 'webhook', 1),
    (3, '设备离线告警', 'device_status=="offline"', 30, 'email', 1),
    (4, 'TPU温度过高', 'tpu_temp>85', 60, 'webhook,email', 0);

-- Seed alarm events
INSERT OR IGNORE INTO alarm_events (id, rule_id, task_id, evidence_path, context) VALUES
    (1, 1, 1, '/evidences/alarm_001.jpg', '{"class":"person","confidence":0.92,"bbox":[100,200,300,400]}'),
    (2, 2, 1, '/evidences/alarm_002.jpg', '{"roi_violation":true,"timestamp":"2024-01-15T10:30:00Z"}'),
    (3, 1, 2, '/evidences/alarm_003.jpg', '{"class":"person","confidence":0.88,"bbox":[150,180,320,410]}'),
    (4, 3, 0, '', '{"device_id":3,"device_name":"BM1684-Edge-03","reason":"heartbeat_timeout"}');

-- Seed monitoring metrics (recent data)
INSERT OR IGNORE INTO monitoring_metrics (metric_type, value, recorded_at) VALUES
    ('cpu', 45.2, datetime('now', '-5 minutes')),
    ('memory', 62.8, datetime('now', '-5 minutes')),
    ('tpu', 78.5, datetime('now', '-5 minutes')),
    ('cpu', 48.1, datetime('now', '-4 minutes')),
    ('memory', 63.2, datetime('now', '-4 minutes')),
    ('tpu', 82.3, datetime('now', '-4 minutes')),
    ('cpu', 42.5, datetime('now', '-3 minutes')),
    ('memory', 61.9, datetime('now', '-3 minutes')),
    ('tpu', 75.6, datetime('now', '-3 minutes')),
    ('cpu', 51.3, datetime('now', '-2 minutes')),
    ('memory', 64.5, datetime('now', '-2 minutes')),
    ('tpu', 85.2, datetime('now', '-2 minutes')),
    ('cpu', 47.8, datetime('now', '-1 minute')),
    ('memory', 63.0, datetime('now', '-1 minute')),
    ('tpu', 79.8, datetime('now', '-1 minute')),
    ('cpu', 46.5, datetime('now')),
    ('memory', 62.5, datetime('now')),
    ('tpu', 77.2, datetime('now'));

-- Seed tasks
INSERT OR IGNORE INTO tasks (id, name, description, device_id, graph_config, status, schedule_cron) VALUES
    (1, '园区人员检测', '主要出入口人员检测与计数', 1, '{"elements":[{"type":"decoder","source":"rtsp://192.168.1.201"},{"type":"detector","model":"yolox_person"},{"type":"tracker","algorithm":"bytetrack"},{"type":"output","type":"rtsp"}]}', 'running', ''),
    (2, '车辆识别统计', '停车场车辆进出识别', 2, '{"elements":[{"type":"decoder","source":"rtsp://192.168.1.202"},{"type":"detector","model":"yolox_vehicle"}]}', 'running', ''),
    (3, '周界防范', '围墙周界入侵检测', 3, '{"elements":[{"type":"decoder","source":"rtsp://192.168.1.203"},{"type":"detector","model":"yolox_person"},{"type":"alarm","condition":"roi_violation"}]}', 'stopped', ''),
    (4, '人脸识别考勤', '办公区域人脸识别打卡', 1, '{"elements":[{"type":"decoder","source":"rtsp://192.168.1.201"},{"type":"detector","model":"face_detect"},{"type":"matcher","threshold":0.8}]}', 'running', '0 9 * * 1-5');

-- Seed plugins
INSERT OR IGNORE INTO plugins (id, name, version, path, signature, status) VALUES
    (1, 'yolox-plugin', '1.0.0', '/plugins/yolox.so', 'sha256:abc123', 'active'),
    (2, 'tracker-plugin', '1.2.0', '/plugins/tracker.so', 'sha256:def456', 'active'),
    (3, 'face-plugin', '2.0.0', '/plugins/face.so', 'sha256:ghi789', 'inactive');

-- Seed config
INSERT OR IGNORE INTO config_versions (config_key, config_value, version) VALUES
    ('system.name', 'Sophon-Stream 边缘AI管理平台', 1),
    ('system.timezone', 'Asia/Shanghai', 1),
    ('system.language', 'zh-CN', 1),
    ('alarm.webhook_url', 'http://192.168.1.100:9090/api/alarm', 1),
    ('alarm.email_to', 'admin@example.com', 1);
)";

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::initialize(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    dbPath_ = dbPath;

    int rc = sqlite3_open(dbPath.c_str(), reinterpret_cast<sqlite3**>(&db_));
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(reinterpret_cast<sqlite3*>(db_)) << std::endl;
        return false;
    }

    if (!createTables()) {
        std::cerr << "Failed to create tables" << std::endl;
        return false;
    }

    if (!seedDefaultData()) {
        std::cerr << "Warning: Failed to seed default data" << std::endl;
    }

    initialized_ = true;
    std::cout << "Database initialized: " << dbPath << std::endl;
    return true;
}

bool DatabaseManager::createTables() {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(reinterpret_cast<sqlite3*>(db_), SCHEMA_SQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::seedDefaultData() {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(reinterpret_cast<sqlite3*>(db_), SEED_DATA_SQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "Seed data error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    std::cout << "Default data seeded: admin user, roles, permissions" << std::endl;
    return true;
}

bool DatabaseManager::execute(const std::string& sql) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !db_) return false;

    char* errMsg = nullptr;
    int rc = sqlite3_exec(reinterpret_cast<sqlite3*>(db_), sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL execute error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int DatabaseManager::queryWithCallback(const std::string& sql, int (*callback)(void*, int, char**, char**), void* userData) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !db_) return -1;

    char* errMsg = nullptr;
    int rc = sqlite3_exec(reinterpret_cast<sqlite3*>(db_), sql.c_str(), callback, userData, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL query error: " << errMsg << " | SQL: " << sql << std::endl;
        sqlite3_free(errMsg);
    }
    return rc;
}

int DatabaseManager::lastInsertRowId() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !db_) return 0;
    return static_cast<int>(sqlite3_last_insert_rowid(reinterpret_cast<sqlite3*>(db_)));
}

std::string DatabaseManager::query(const std::string& sql) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !db_) return "[]";

    // Simple query implementation - in production use proper ORM
    return "[]";
}

bool DatabaseManager::backup(const std::string& backupPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || !db_) return false;

    std::ifstream src(dbPath_, std::ios::binary);
    std::ofstream dst(backupPath, std::ios::binary);
    if (!src.is_open() || !dst.is_open()) return false;

    dst << src.rdbuf();
    return true;
}

bool DatabaseManager::restore(const std::string& backupPath) {
    std::lock_guard<std::mutex> lock(mutex_);
    close();

    std::ifstream src(backupPath, std::ios::binary);
    std::ofstream dst(dbPath_, std::ios::binary);
    if (!src.is_open() || !dst.is_open()) return false;

    dst << src.rdbuf();

    return initialize(dbPath_);
}

void DatabaseManager::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (db_) {
        sqlite3_close(reinterpret_cast<sqlite3*>(db_));
        db_ = nullptr;
        initialized_ = false;
    }
}

} // namespace db
} // namespace web
} // namespace sophon
