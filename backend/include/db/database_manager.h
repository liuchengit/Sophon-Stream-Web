#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <functional>

namespace sophon {
namespace web {
namespace db {

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool initialize(const std::string& dbPath = "data/sophon-web.db");
    bool execute(const std::string& sql);
    int queryWithCallback(const std::string& sql, int (*callback)(void*, int, char**, char**), void* userData);
    int lastInsertRowId();
    std::string query(const std::string& sql);

    bool backup(const std::string& backupPath);
    bool restore(const std::string& backupPath);

    void close();
    void* db() const { return db_; }

private:
    DatabaseManager() = default;
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createTables();
    bool seedDefaultData();

    void* db_;
    std::mutex mutex_;
    std::string dbPath_;
    bool initialized_ = false;
};

} // namespace db
} // namespace web
} // namespace sophon
