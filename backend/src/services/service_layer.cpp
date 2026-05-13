#include "services/service_layer.h"
#include "db/repositories.h"
#include "db/transaction.h"
#include "db/database_manager.h"
#include "stream_engine.h"
#include "graph_manager.h"
#include "result_collector.h"
#include <iostream>

namespace sophon {
namespace web {
namespace services {

// DeviceService

std::optional<models::Device> DeviceService::getDevice(int id) {
    return db::DeviceRepository::findById(id);
}

std::vector<models::Device> DeviceService::listDevices(const std::string& status, const std::string& type, int page, int limit) {
    return db::DeviceRepository::findAll(status, type, page, limit);
}

int DeviceService::countDevices(const std::string& status, const std::string& type) {
    return db::DeviceRepository::count(status, type);
}

int DeviceService::createDevice(const models::Device& device) {
    return db::DeviceRepository::create(device);
}

bool DeviceService::updateDevice(int id, const models::Device& device) {
    return db::DeviceRepository::update(id, device);
}

bool DeviceService::deleteDevice(int id) {
    return db::DeviceRepository::remove(id);
}

bool DeviceService::updateDeviceStatus(int id, const std::string& status) {
    return db::DeviceRepository::updateStatus(id, status);
}

bool DeviceService::checkDeviceOnline(const std::string& /*ipAddress*/, int /*port*/) {
    // In production: ping or HTTP health check
    return false;
}

// TaskService

std::optional<models::Task> TaskService::getTask(int id) {
    return db::TaskRepository::findById(id);
}

std::vector<models::Task> TaskService::listTasks(const std::string& status, int page, int limit) {
    return db::TaskRepository::findAll(status, page, limit);
}

int TaskService::countTasks(const std::string& status) {
    return db::TaskRepository::count(status);
}

int TaskService::createTask(const models::Task& task) {
    return db::TaskRepository::create(task);
}

bool TaskService::updateTask(int id, const models::Task& task) {
    return db::TaskRepository::update(id, task);
}

bool TaskService::deleteTask(int id) {
    return db::TaskRepository::remove(id);
}

bool TaskService::updateTaskStatus(int id, const std::string& status) {
    return db::TaskRepository::updateStatus(id, status);
}

bool TaskService::startTask(int taskId) {
    auto task = db::TaskRepository::findById(taskId);
    if (!task) return false;

    if (task->status == "running") {
        std::cerr << "Task " << taskId << " is already running" << std::endl;
        return false;
    }

    // Create and start graph
    sophon::stream::GraphConfig config;
    config.name = task->name;

    // Parse graph config
    try {
        if (!task->graph_config.empty()) {
            config.config = nlohmann::json::parse(task->graph_config);
            if (config.config.contains("elements")) {
                for (const auto& elem : config.config["elements"]) {
                    config.elements.push_back(elem.value("type", ""));
                }
            }
        }
    } catch (...) {
        config.elements = {"decoder", "detector", "tracker"};
    }

    auto graph = sophon::stream::GraphManager::instance().createGraph(config);
    if (!graph) {
        std::cerr << "Failed to create graph for task " << taskId << std::endl;
        return false;
    }

    return db::TaskRepository::updateStatus(taskId, "running");
}

bool TaskService::stopTask(int taskId) {
    auto graph = sophon::stream::GraphManager::instance().getGraph(taskId);
    if (graph) {
        graph->stop();
        sophon::stream::GraphManager::instance().destroyGraph(taskId);
    }
    return db::TaskRepository::updateStatus(taskId, "stopped");
}

bool TaskService::pauseTask(int taskId) {
    auto graph = sophon::stream::GraphManager::instance().getGraph(taskId);
    if (!graph) return false;
    return graph->pause();
}

bool TaskService::resumeTask(int taskId) {
    auto graph = sophon::stream::GraphManager::instance().getGraph(taskId);
    if (!graph) return false;
    return graph->resume();
}

bool TaskService::updateTaskConfig(int taskId, const std::string& config) {
    auto graph = sophon::stream::GraphManager::instance().getGraph(taskId);
    if (!graph) return false;

    try {
        nlohmann::json newConfig = nlohmann::json::parse(config);
        return graph->updateConfig(newConfig);
    } catch (...) {
        return false;
    }
}

// AlgorithmService

std::optional<models::Algorithm> AlgorithmService::getAlgorithm(int id) {
    return db::AlgorithmRepository::findById(id);
}

std::vector<models::Algorithm> AlgorithmService::listAlgorithms(int page, int limit) {
    return db::AlgorithmRepository::findAll(page, limit);
}

int AlgorithmService::countAlgorithms() {
    return db::AlgorithmRepository::count();
}

int AlgorithmService::createAlgorithm(const models::Algorithm& algo) {
    return db::AlgorithmRepository::create(algo);
}

bool AlgorithmService::updateAlgorithm(int id, const models::Algorithm& algo) {
    return db::AlgorithmRepository::update(id, algo);
}

bool AlgorithmService::deleteAlgorithm(int id) {
    return db::AlgorithmRepository::remove(id);
}

bool AlgorithmService::installAlgorithm(const std::string& modelPath, const std::string& pluginPath) {
    models::Algorithm algo;
    algo.model_path = modelPath;
    algo.plugin_path = pluginPath;
    algo.status = "installed";
    return db::AlgorithmRepository::create(algo) > 0;
}

bool AlgorithmService::uninstallAlgorithm(int id) {
    return db::AlgorithmRepository::remove(id);
}

// AlarmService

std::optional<models::AlarmRule> AlarmService::getRule(int id) {
    return db::AlarmRuleRepository::findById(id);
}

std::vector<models::AlarmRule> AlarmService::listRules(bool enabledOnly) {
    return db::AlarmRuleRepository::findAll(enabledOnly);
}

int AlarmService::createRule(const models::AlarmRule& rule) {
    return db::AlarmRuleRepository::create(rule);
}

bool AlarmService::updateRule(int id, const models::AlarmRule& rule) {
    return db::AlarmRuleRepository::update(id, rule);
}

bool AlarmService::deleteRule(int id) {
    return db::AlarmRuleRepository::remove(id);
}

bool AlarmService::toggleRule(int id, bool enabled) {
    return db::AlarmRuleRepository::toggle(id, enabled);
}

std::optional<models::AlarmEvent> AlarmService::getEvent(int id) {
    return db::AlarmEventRepository::findById(id);
}

std::vector<models::AlarmEvent> AlarmService::listEvents(int page, int limit) {
    return db::AlarmEventRepository::findAll(page, limit);
}

int AlarmService::countEvents() {
    return db::AlarmEventRepository::count();
}

int AlarmService::createEvent(const models::AlarmEvent& event) {
    return db::AlarmEventRepository::create(event);
}

// ConfigService

std::optional<models::ConfigVersion> ConfigService::getLatestConfig(const std::string& key) {
    return db::ConfigVersionRepository::findLatest(key);
}

std::vector<models::ConfigVersion> ConfigService::getConfigHistory(const std::string& key, int limit) {
    return db::ConfigVersionRepository::findHistory(key, limit);
}

int ConfigService::updateConfig(const std::string& key, const std::string& value) {
    int version = db::ConfigVersionRepository::create(key, value);

    // Hot-update engine config if applicable
    if (key == "engine") {
        int newVersion = 0;
        try {
            nlohmann::json config = nlohmann::json::parse(value);
            sophon::stream::StreamEngine::instance().updateConfig(config, newVersion);
        } catch (...) {
            // Rollback on failure
            db::ConfigVersionRepository::rollback(key, version - 1);
        }
    }

    return version;
}

bool ConfigService::rollbackConfig(const std::string& key, int version) {
    return db::ConfigVersionRepository::rollback(key, version);
}

// UpgradeService

std::optional<models::UpgradeRecord> UpgradeService::getRecord(int id) {
    return db::UpgradeRecordRepository::findById(id);
}

std::vector<models::UpgradeRecord> UpgradeService::listRecords(int page, int limit) {
    return db::UpgradeRecordRepository::findAll(page, limit);
}

int UpgradeService::createUpgrade(const models::UpgradeRecord& record) {
    return db::UpgradeRecordRepository::create(record);
}

bool UpgradeService::updateUpgrade(int id, const models::UpgradeRecord& record) {
    return db::UpgradeRecordRepository::update(id, record);
}

bool UpgradeService::executeUpgrade(int /*recordId*/) {
    // In production: download, verify, apply upgrade package
    return true;
}

bool UpgradeService::rollbackUpgrade(int /*recordId*/) {
    // In production: rollback to previous version
    return true;
}

// PluginService

std::optional<models::Plugin> PluginService::getPlugin(int id) {
    return db::PluginRepository::findById(id);
}

std::optional<models::Plugin> PluginService::getPluginByName(const std::string& name) {
    return db::PluginRepository::findByName(name);
}

std::vector<models::Plugin> PluginService::listPlugins(const std::string& status) {
    return db::PluginRepository::findAll(status);
}

int PluginService::createPlugin(const models::Plugin& plugin) {
    return db::PluginRepository::create(plugin);
}

bool PluginService::updatePlugin(int id, const models::Plugin& plugin) {
    return db::PluginRepository::update(id, plugin);
}

bool PluginService::deletePlugin(int id) {
    return db::PluginRepository::remove(id);
}

bool PluginService::enablePlugin(int id) {
    auto plugin = db::PluginRepository::findById(id);
    if (!plugin) return false;
    plugin->status = "active";
    return db::PluginRepository::update(id, *plugin);
}

bool PluginService::disablePlugin(int id) {
    auto plugin = db::PluginRepository::findById(id);
    if (!plugin) return false;
    plugin->status = "inactive";
    return db::PluginRepository::update(id, *plugin);
}

} // namespace services
} // namespace web
} // namespace sophon
