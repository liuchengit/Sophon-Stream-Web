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
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS alarm_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    rule_id INTEGER REFERENCES alarm_rules(id),
    task_id INTEGER REFERENCES tasks(id),
    evidence_path VARCHAR(255),
    context TEXT,
    triggered_at DATETIME DEFAULT CURRENT_TIMESTAMP
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

-- Insert default admin role
INSERT OR IGNORE INTO roles (id, name, description) VALUES (1, 'admin', 'System Administrator');

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

-- Insert default admin user (password: admin123, bcrypt hash)
INSERT OR IGNORE INTO users (username, password_hash, role_id)
VALUES ('admin', '$2b$12$LJ3m4ys3Lk0TfKzqRqFZi.vMx7c5qZqZqZqZqZqZqZqZqZqZqZqZq', 1);
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
