#include "utils/logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <filesystem>

namespace sophon {
namespace web {

LogManager& LogManager::instance() {
    static LogManager instance;
    return instance;
}

bool LogManager::initialize(const std::string& logPath, LogLevel level, size_t maxSize) {
    logPath_ = logPath;
    level_ = level;
    maxSize_ = maxSize;
    currentSize_ = 0;

    std::filesystem::create_directories(logPath);

    std::cout << "Log manager initialized: " << logPath_ << " (level: " << levelToString(level) << ")" << std::endl;
    return true;
}

void LogManager::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

void LogManager::debug(const std::string& module, const std::string& message) {
    if (level_ > LogLevel::DEBUG) return;
    writeToFile(LogLevel::DEBUG, module, message);
}

void LogManager::info(const std::string& module, const std::string& message) {
    if (level_ > LogLevel::INFO) return;
    writeToFile(LogLevel::INFO, module, message);
}

void LogManager::warning(const std::string& module, const std::string& message) {
    if (level_ > LogLevel::WARNING) return;
    writeToFile(LogLevel::WARNING, module, message);
}

void LogManager::error(const std::string& module, const std::string& message) {
    writeToFile(LogLevel::ERROR, module, message);
}

void LogManager::logOperation(const std::string& user, const std::string& operation, const std::string& detail) {
    std::string msg = "User=" + user + " Operation=" + operation + " Detail=" + detail;
    writeToFile(LogLevel::INFO, "AUDIT", msg);
}

void LogManager::writeToFile(LogLevel level, const std::string& module, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string logFile = logPath_ + "/sophon-web.log";

    // Check if rotation needed
    if (currentSize_ >= maxSize_) {
        rotateLogFile();
    }

    std::ofstream file(logFile, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open log file: " << logFile << std::endl;
        // Fallback to console
        std::cout << "[" << levelToString(level) << "] [" << module << "] " << message << std::endl;
        return;
    }

    file << "[" << getTimestamp() << "] [" << levelToString(level) << "] [" << module << "] " << message << std::endl;
    currentSize_ += message.size() + 50; // Approximate

    // Also output to console for ERROR and WARNING
    if (level >= LogLevel::WARNING) {
        std::cout << "[" << levelToString(level) << "] [" << module << "] " << message << std::endl;
    }
}

void LogManager::rotateLogFile() {
    std::string oldFile = logPath_ + "/sophon-web.log";
    std::string newFile = logPath_ + "/sophon-web.log.old";

    // Move current log to old
    std::filesystem::rename(oldFile, newFile);
    currentSize_ = 0;

    // Clean old logs older than 7 days
    auto now = std::filesystem::file_time_type::clock::now();
    for (const auto& entry : std::filesystem::directory_iterator(logPath_)) {
        if (entry.path().extension() == ".log.old") {
            auto lastWrite = std::filesystem::last_write_time(entry.path());
            auto age = std::chrono::duration_cast<std::chrono::hours>(now - lastWrite).count();
            if (age > 24 * 7) {
                std::filesystem::remove(entry.path());
            }
        }
    }
}

std::string LogManager::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string LogManager::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

} // namespace web
} // namespace sophon
