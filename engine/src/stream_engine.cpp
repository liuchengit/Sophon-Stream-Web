#include "stream_engine.h"
#include <fstream>
#include <iostream>

namespace sophon {
namespace stream {

StreamEngine& StreamEngine::instance() {
    static StreamEngine instance;
    return instance;
}

bool StreamEngine::initialize(const std::string& configPath) {
    std::cout << "Initializing Sophon-Stream Engine..." << std::endl;

    if (!loadConfig(configPath)) {
        std::cerr << "Failed to load config from: " << configPath << std::endl;
        return false;
    }

    running_ = true;
    currentVersion_ = 1;
    std::cout << "Sophon-Stream Engine initialized successfully." << std::endl;
    return true;
}

bool StreamEngine::loadConfig(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        // Use default config
        config_ = {
            {"graphs", json::array()},
            {"elements", {
                {"decoder", {{"type", "decoder"}, {"codec", "h264"}}},
                {"detector", {{"type", "detector"}, {"model", "yolox"}}},
                {"tracker", {{"type", "tracker"}, {"algorithm", "bytetrack"}}},
            }}
        };
        return true;
    }

    try {
        file >> config_;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Config parse error: " << e.what() << std::endl;
        return false;
    }
}

bool StreamEngine::updateConfig(const json& newConfig, int& newVersion) {
    if (!running_) {
        std::cerr << "Engine not running" << std::endl;
        return false;
    }

    // Validate new config
    if (!newConfig.contains("graphs")) {
        std::cerr << "Invalid config: missing graphs" << std::endl;
        return false;
    }

    // Apply new config (in production, this would hot-update running graphs)
    config_ = newConfig;
    newVersion = ++currentVersion_;

    std::cout << "Config updated to version " << newVersion << std::endl;
    return true;
}

void StreamEngine::shutdown() {
    running_ = false;
    std::cout << "Sophon-Stream Engine shutdown." << std::endl;
}

} // namespace stream
} // namespace sophon
