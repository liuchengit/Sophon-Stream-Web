#pragma once

#include "models/entities.h"
#include <vector>
#include <string>
#include <optional>

namespace sophon {
namespace web {
namespace services {

class DeviceService {
public:
    static std::optional<models::Device> getDevice(int id);
    static std::vector<models::Device> listDevices(const std::string& status = "", const std::string& type = "", int page = 1, int limit = 20);
    static int countDevices(const std::string& status = "", const std::string& type = "");
    static int createDevice(const models::Device& device);
    static bool updateDevice(int id, const models::Device& device);
    static bool deleteDevice(int id);
    static bool updateDeviceStatus(int id, const std::string& status);

private:
    static bool checkDeviceOnline(const std::string& ipAddress, int port);
};

class TaskService {
public:
    static std::optional<models::Task> getTask(int id);
    static std::vector<models::Task> listTasks(const std::string& status = "", int page = 1, int limit = 20);
    static int countTasks(const std::string& status = "");
    static int createTask(const models::Task& task);
    static bool updateTask(int id, const models::Task& task);
    static bool deleteTask(int id);
    static bool updateTaskStatus(int id, const std::string& status);

    static bool startTask(int taskId);
    static bool stopTask(int taskId);
    static bool pauseTask(int taskId);
    static bool resumeTask(int taskId);
    static bool updateTaskConfig(int taskId, const std::string& config);
};

class AlgorithmService {
public:
    static std::optional<models::Algorithm> getAlgorithm(int id);
    static std::vector<models::Algorithm> listAlgorithms(int page = 1, int limit = 20);
    static int countAlgorithms();
    static int createAlgorithm(const models::Algorithm& algo);
    static bool updateAlgorithm(int id, const models::Algorithm& algo);
    static bool deleteAlgorithm(int id);
    static bool installAlgorithm(const std::string& modelPath, const std::string& pluginPath);
    static bool uninstallAlgorithm(int id);
};

class AlarmService {
public:
    static std::optional<models::AlarmRule> getRule(int id);
    static std::vector<models::AlarmRule> listRules(bool enabledOnly = false);
    static int createRule(const models::AlarmRule& rule);
    static bool updateRule(int id, const models::AlarmRule& rule);
    static bool deleteRule(int id);
    static bool toggleRule(int id, bool enabled);

    static std::optional<models::AlarmEvent> getEvent(int id);
    static std::vector<models::AlarmEvent> listEvents(int page = 1, int limit = 20);
    static int countEvents();
    static int createEvent(const models::AlarmEvent& event);
};

class ConfigService {
public:
    static std::optional<models::ConfigVersion> getLatestConfig(const std::string& key);
    static std::vector<models::ConfigVersion> getConfigHistory(const std::string& key, int limit = 10);
    static int updateConfig(const std::string& key, const std::string& value);
    static bool rollbackConfig(const std::string& key, int version);
};

class UpgradeService {
public:
    static std::optional<models::UpgradeRecord> getRecord(int id);
    static std::vector<models::UpgradeRecord> listRecords(int page = 1, int limit = 20);
    static int createUpgrade(const models::UpgradeRecord& record);
    static bool updateUpgrade(int id, const models::UpgradeRecord& record);
    static bool executeUpgrade(int recordId);
    static bool rollbackUpgrade(int recordId);
};

class PluginService {
public:
    static std::optional<models::Plugin> getPlugin(int id);
    static std::optional<models::Plugin> getPluginByName(const std::string& name);
    static std::vector<models::Plugin> listPlugins(const std::string& status = "");
    static int createPlugin(const models::Plugin& plugin);
    static bool updatePlugin(int id, const models::Plugin& plugin);
    static bool deletePlugin(int id);
    static bool enablePlugin(int id);
    static bool disablePlugin(int id);
};

} // namespace services
} // namespace web
} // namespace sophon
