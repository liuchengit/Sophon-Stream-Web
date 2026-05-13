#pragma once

#include <string>
#include <memory>
#include <mutex>

namespace sophon {
namespace web {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class LogManager {
public:
    static LogManager& instance();

    bool initialize(const std::string& logPath = "logs", LogLevel level = LogLevel::INFO, size_t maxSize = 100 * 1024 * 1024);
    void setLevel(LogLevel level);

    void debug(const std::string& module, const std::string& message);
    void info(const std::string& module, const std::string& message);
    void warning(const std::string& module, const std::string& message);
    void error(const std::string& module, const std::string& message);

    void logOperation(const std::string& user, const std::string& operation, const std::string& detail);

private:
    LogManager() = default;

    void writeToFile(LogLevel level, const std::string& module, const std::string& message);
    void rotateLogFile();
    std::string levelToString(LogLevel level) const;
    std::string getTimestamp() const;

    std::string logPath_;
    LogLevel level_;
    size_t maxSize_;
    size_t currentSize_;
    std::mutex mutex_;
};

#define LOG_DEBUG(module, msg) sophon::web::LogManager::instance().debug(module, msg)
#define LOG_INFO(module, msg) sophon::web::LogManager::instance().info(module, msg)
#define LOG_WARNING(module, msg) sophon::web::LogManager::instance().warning(module, msg)
#define LOG_ERROR(module, msg) sophon::web::LogManager::instance().error(module, msg)

} // namespace web
} // namespace sophon
