#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace sophon {
namespace stream {

using json = nlohmann::json;

class StreamEngine {
public:
    static StreamEngine& instance();

    bool initialize(const std::string& configPath = "engine/configs/default.json");
    bool updateConfig(const json& newConfig, int& newVersion);
    void shutdown();

    bool isRunning() const { return running_; }

private:
    StreamEngine() = default;
    ~StreamEngine() = default;

    StreamEngine(const StreamEngine&) = delete;
    StreamEngine& operator=(const StreamEngine&) = delete;

    bool loadConfig(const std::string& configPath);

    bool running_ = false;
    int currentVersion_ = 0;
    json config_;
};

} // namespace stream
} // namespace sophon
