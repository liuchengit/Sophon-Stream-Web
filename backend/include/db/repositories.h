#pragma once

#include "models/entities.h"
#include <vector>
#include <string>
#include <optional>

namespace sophon {
namespace web {
namespace db {

class UserRepository {
public:
    static std::optional<models::User> findByUsername(const std::string& username);
    static std::optional<models::User> findById(int id);
    static std::vector<models::User> findAll(int page = 1, int limit = 20);
    static int create(const models::User& user);
    static bool update(int id, const models::User& user);
    static bool remove(int id);
};

class DeviceRepository {
public:
    static std::optional<models::Device> findById(int id);
    static std::vector<models::Device> findAll(const std::string& status = "", const std::string& type = "", int page = 1, int limit = 20);
    static int count(const std::string& status = "", const std::string& type = "");
    static int create(const models::Device& device);
    static bool update(int id, const models::Device& device);
    static bool remove(int id);
    static bool updateStatus(int id, const std::string& status);
};

class TaskRepository {
public:
    static std::optional<models::Task> findById(int id);
    static std::vector<models::Task> findAll(const std::string& status = "", int page = 1, int limit = 20);
    static int count(const std::string& status = "");
    static int create(const models::Task& task);
    static bool update(int id, const models::Task& task);
    static bool remove(int id);
    static bool updateStatus(int id, const std::string& status);
};

class AlgorithmRepository {
public:
    static std::optional<models::Algorithm> findById(int id);
    static std::vector<models::Algorithm> findAll(int page = 1, int limit = 20);
    static int count();
    static int create(const models::Algorithm& algo);
    static bool update(int id, const models::Algorithm& algo);
    static bool remove(int id);
};

class AlarmRuleRepository {
public:
    static std::optional<models::AlarmRule> findById(int id);
    static std::vector<models::AlarmRule> findAll(bool enabledOnly = false);
    static int create(const models::AlarmRule& rule);
    static bool update(int id, const models::AlarmRule& rule);
    static bool remove(int id);
    static bool toggle(int id, bool enabled);
    static bool updateSubscribeStatus(int id, const std::string& status, const std::string& expires);
};

class AlarmEventRepository {
public:
    static std::optional<models::AlarmEvent> findById(int id);
    static std::vector<models::AlarmEvent> findAll(int page = 1, int limit = 20);
    static int count();
    static int create(const models::AlarmEvent& event);
    static bool updateHandled(int id, int userId, const std::string& result);
};

class AlarmSubscriptionRepository {
public:
    static std::optional<models::AlarmSubscription> findById(int id);
    static std::vector<models::AlarmSubscription> findByDevice(int deviceId);
    static std::vector<models::AlarmSubscription> findAll();
    static int create(const models::AlarmSubscription& sub);
    static bool update(int id, const models::AlarmSubscription& sub);
    static bool remove(int id);
    static bool updateHeartbeat(int id);
};

class MonitoringMetricRepository {
public:
    static int create(const models::MonitoringMetric& metric);
    static std::vector<models::MonitoringMetric> findByType(const std::string& type, const std::string& startTime, const std::string& endTime);
    static bool cleanOldMetrics(int daysToKeep = 7);
};

class ConfigVersionRepository {
public:
    static std::optional<models::ConfigVersion> findLatest(const std::string& key);
    static std::vector<models::ConfigVersion> findHistory(const std::string& key, int limit = 10);
    static int create(const std::string& key, const std::string& value);
    static bool rollback(const std::string& key, int version);
};

class PluginRepository {
public:
    static std::optional<models::Plugin> findById(int id);
    static std::optional<models::Plugin> findByName(const std::string& name);
    static std::vector<models::Plugin> findAll(const std::string& status = "");
    static int create(const models::Plugin& plugin);
    static bool update(int id, const models::Plugin& plugin);
    static bool remove(int id);
};

class UpgradeRecordRepository {
public:
    static std::optional<models::UpgradeRecord> findById(int id);
    static std::vector<models::UpgradeRecord> findAll(int page = 1, int limit = 20);
    static int create(const models::UpgradeRecord& record);
    static bool update(int id, const models::UpgradeRecord& record);
};

class WorkflowRepository {
public:
    static std::optional<models::Workflow> findById(int id);
    static std::vector<models::Workflow> findAll(const std::string& status = "", int page = 1, int limit = 20);
    static int count(const std::string& status = "");
    static int create(const models::Workflow& workflow);
    static bool update(int id, const models::Workflow& workflow);
    static bool remove(int id);
    static bool updateStatus(int id, const std::string& status);
};

class WorkflowNodeRepository {
public:
    static std::vector<models::WorkflowNode> findByWorkflowId(int workflowId);
    static int create(const models::WorkflowNode& node);
    static bool update(int workflowId, const std::string& nodeId, const models::WorkflowNode& node);
    static bool removeByWorkflowId(int workflowId);
};

class WorkflowEdgeRepository {
public:
    static std::vector<models::WorkflowEdge> findByWorkflowId(int workflowId);
    static int create(const models::WorkflowEdge& edge);
    static bool removeByWorkflowId(int workflowId);
};

class WorkflowExecutionRepository {
public:
    static std::optional<models::WorkflowExecution> findById(int id);
    static std::vector<models::WorkflowExecution> findByWorkflowId(int workflowId, int limit = 20);
    static std::optional<models::WorkflowExecution> findLatestRunning(int workflowId);
    static int create(int workflowId);
    static bool updateStatus(int id, const std::string& status, const std::string& errorMsg = "");
    static bool finish(int id, const std::string& errorMsg = "");
};

class WorkflowExecutionNodeRepository {
public:
    static std::vector<models::WorkflowExecutionNode> findByExecutionId(int executionId);
    static std::optional<models::WorkflowExecutionNode> findByExecutionAndNodeId(int executionId, const std::string& nodeId);
    static int create(int executionId, const std::string& nodeId, const std::string& nodeType, const std::string& label);
    static bool updateStatus(int executionId, const std::string& nodeId, const std::string& status, const std::string& errorMsg = "");
};

} // namespace db
} // namespace web
} // namespace sophon
