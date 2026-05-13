#include "graph_manager.h"
#include <iostream>
#include <algorithm>

namespace sophon {
namespace stream {

// GraphInstance implementation

GraphInstance::GraphInstance(int id) : id_(id) {}

GraphInstance::~GraphInstance() {
    stop();
}

bool GraphInstance::start(const GraphConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_ == GraphStatus::RUNNING) {
        std::cerr << "Graph " << id_ << " is already running" << std::endl;
        return false;
    }

    config_ = config;
    name_ = config.name;

    // In production: create and start sophon-stream graph
    std::cout << "Starting graph " << id_ << ": " << name_ << std::endl;
    std::cout << "  Elements: ";
    for (const auto& elem : config.elements) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    status_ = GraphStatus::RUNNING;
    return true;
}

bool GraphInstance::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_ == GraphStatus::STOPPED) return true;

    std::cout << "Stopping graph " << id_ << std::endl;
    status_ = GraphStatus::STOPPED;
    return true;
}

bool GraphInstance::pause() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_ != GraphStatus::RUNNING) return false;

    std::cout << "Pausing graph " << id_ << std::endl;
    status_ = GraphStatus::PAUSED;
    return true;
}

bool GraphInstance::resume() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_ != GraphStatus::PAUSED) return false;

    std::cout << "Resuming graph " << id_ << std::endl;
    status_ = GraphStatus::RUNNING;
    return true;
}

bool GraphInstance::updateConfig(const json& newConfig) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_ == GraphStatus::ERROR) return false;

    // In production: hot-update graph configuration
    std::cout << "Updating config for graph " << id_ << std::endl;
    config_.config = newConfig;
    return true;
}

// GraphManager implementation

GraphManager& GraphManager::instance() {
    static GraphManager instance;
    return instance;
}

std::shared_ptr<GraphInstance> GraphManager::createGraph(const GraphConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    int id = nextId_++;
    auto graph = std::make_shared<GraphInstance>(id);

    if (!graph->start(config)) {
        return nullptr;
    }

    graphs_[id] = graph;
    std::cout << "Created graph " << id << ": " << config.name << std::endl;
    return graph;
}

bool GraphManager::destroyGraph(int graphId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = graphs_.find(graphId);
    if (it == graphs_.end()) return false;

    it->second->stop();
    graphs_.erase(it);
    std::cout << "Destroyed graph " << graphId << std::endl;
    return true;
}

std::shared_ptr<GraphInstance> GraphManager::getGraph(int graphId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = graphs_.find(graphId);
    if (it == graphs_.end()) return nullptr;
    return it->second;
}

std::vector<std::shared_ptr<GraphInstance>> GraphManager::listGraphs() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<GraphInstance>> result;
    for (const auto& pair : graphs_) {
        result.push_back(pair.second);
    }
    return result;
}

int GraphManager::getActiveGraphCount() const {
    int count = 0;
    for (const auto& pair : graphs_) {
        if (pair.second->getStatus() == GraphStatus::RUNNING) count++;
    }
    return count;
}

void GraphManager::stopAllGraphs() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : graphs_) {
        pair.second->stop();
    }
}

} // namespace stream
} // namespace sophon
