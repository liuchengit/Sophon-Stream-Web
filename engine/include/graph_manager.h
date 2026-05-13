#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>

namespace sophon {
namespace stream {

using json = nlohmann::json;

enum class GraphStatus {
    CREATED,
    RUNNING,
    PAUSED,
    STOPPED,
    ERROR
};

struct GraphConfig {
    std::string name;
    std::vector<std::string> elements;
    json config;
};

class GraphInstance {
public:
    explicit GraphInstance(int id);
    ~GraphInstance();

    bool start(const GraphConfig& config);
    bool stop();
    bool pause();
    bool resume();
    bool updateConfig(const json& newConfig);

    int getId() const { return id_; }
    GraphStatus getStatus() const { return status_; }
    const std::string& getName() const { return name_; }

private:
    int id_;
    std::string name_;
    GraphStatus status_ = GraphStatus::CREATED;
    GraphConfig config_;
    std::mutex mutex_;
};

class GraphManager {
public:
    static GraphManager& instance();

    std::shared_ptr<GraphInstance> createGraph(const GraphConfig& config);
    bool destroyGraph(int graphId);
    std::shared_ptr<GraphInstance> getGraph(int graphId);
    std::vector<std::shared_ptr<GraphInstance>> listGraphs();

    int getActiveGraphCount() const;
    void stopAllGraphs();

private:
    GraphManager() = default;
    ~GraphManager() = default;

    std::map<int, std::shared_ptr<GraphInstance>> graphs_;
    std::mutex mutex_;
    int nextId_ = 1;
};

} // namespace stream
} // namespace sophon
