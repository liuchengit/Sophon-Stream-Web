#include <iostream>
#include <cassert>
#include <filesystem>
#include "db/database_manager.h"
#include "db/repositories.h"
#include "models/entities.h"

using namespace sophon::web;
using namespace sophon::web::db;
using namespace sophon::web::models;

static const std::string TEST_DB = "test_sophon_web.db";

class TestRunner {
public:
    int passed = 0;
    int failed = 0;

    void run(const std::string& name, auto test) {
        try {
            test();
            std::cout << "  PASS: " << name << std::endl;
            passed++;
        } catch (const std::exception& e) {
            std::cout << "  FAIL: " << name << " - " << e.what() << std::endl;
            failed++;
        }
    }
};

void testDatabaseInitialization(TestRunner& runner) {
    runner.run("Database initializes successfully", []() {
        std::filesystem::remove(TEST_DB);
        bool result = DatabaseManager::instance().initialize(TEST_DB);
        assert(result == true);
    });
}

void testDeviceCRUD(TestRunner& runner) {
    runner.run("Device create returns positive ID", []() {
        Device device;
        device.name = "Test Camera";
        device.type = "camera";
        device.ip_address = "192.168.1.50";
        device.port = 554;
        device.status = "offline";
        device.model = "Test Model";

        int id = DeviceRepository::create(device);
        assert(id >= 0);
    });

    runner.run("Device find by ID returns data", []() {
        auto device = DeviceRepository::findById(1);
        assert(device.has_value());
        assert(device->name == "Test Camera");
        assert(device->type == "camera");
    });

    runner.run("Device update changes fields", []() {
        Device device;
        device.name = "Updated Camera";
        device.type = "camera";
        device.ip_address = "192.168.1.51";
        device.port = 554;
        device.status = "online";
        device.model = "New Model";

        bool result = DeviceRepository::update(1, device);
        assert(result == true);

        auto updated = DeviceRepository::findById(1);
        assert(updated.has_value());
        assert(updated->name == "Updated Camera");
    });

    runner.run("Device delete removes record", []() {
        bool result = DeviceRepository::remove(1);
        assert(result == true);
    });

    runner.run("Device update status works", []() {
        Device device;
        device.name = "Status Test Camera";
        device.type = "camera";
        device.ip_address = "192.168.1.52";
        device.port = 554;

        int id = DeviceRepository::create(device);
        assert(id >= 0);

        bool result = DeviceRepository::updateStatus(id, "online");
        assert(result == true);
    });
}

void testTaskCRUD(TestRunner& runner) {
    runner.run("Task create returns positive ID", []() {
        Task task;
        task.name = "Test Task";
        task.description = "Test description";
        task.device_id = 1;
        task.status = "stopped";
        task.graph_config = R"({"elements":["decoder","detector"]})";

        int id = TaskRepository::create(task);
        assert(id >= 0);
    });

    runner.run("Task update status works", []() {
        bool result = TaskRepository::updateStatus(1, "running");
        assert(result == true);

        auto task = TaskRepository::findById(1);
        assert(task.has_value());
        assert(task->status == "running");
    });
}

void testAlgorithmCRUD(TestRunner& runner) {
    runner.run("Algorithm create returns positive ID", []() {
        Algorithm algo;
        algo.name = "YOLOX";
        algo.version = "1.0.0";
        algo.type = "detection";
        algo.model_path = "/models/yolox.bmodel";

        int id = AlgorithmRepository::create(algo);
        assert(id >= 0);
    });

    runner.run("Algorithm list returns data", []() {
        auto algos = AlgorithmRepository::findAll();
        assert(!algos.empty());
        assert(algos[0].name == "YOLOX");
    });
}

void testAlarmRuleCRUD(TestRunner& runner) {
    runner.run("Alarm rule create returns positive ID", []() {
        AlarmRule rule;
        rule.name = "High Confidence Alert";
        rule.condition_expr = "confidence>0.9";
        rule.debounce_seconds = 30;
        rule.enabled = true;

        int id = AlarmRuleRepository::create(rule);
        assert(id >= 0);
    });

    runner.run("Alarm rule toggle works", []() {
        bool result = AlarmRuleRepository::toggle(1, false);
        assert(result == true);
    });
}

void testAlarmEventCRUD(TestRunner& runner) {
    runner.run("Alarm event create returns positive ID", []() {
        AlarmEvent event;
        event.rule_id = 1;
        event.task_id = 1;
        event.evidence_path = "/evidence/frame_001.jpg";
        event.context = R"({"confidence":0.95})";

        int id = AlarmEventRepository::create(event);
        assert(id >= 0);
    });
}

void testMonitoringMetricCRUD(TestRunner& runner) {
    runner.run("Monitoring metric create returns positive ID", []() {
        MonitoringMetric metric;
        metric.metric_type = "cpu";
        metric.value = 45.2;

        int id = MonitoringMetricRepository::create(metric);
        assert(id >= 0);
    });

    runner.run("Monitoring metric clean old works", []() {
        bool result = MonitoringMetricRepository::cleanOldMetrics(7);
        assert(result == true);
    });
}

void testConfigVersionCRUD(TestRunner& runner) {
    runner.run("Config version create increments version", []() {
        int v1 = ConfigVersionRepository::create("engine", R"({"maxGraphs":8})");
        int v2 = ConfigVersionRepository::create("engine", R"({"maxGraphs":16})");
        assert(v2 > v1);
    });
}

void testPluginCRUD(TestRunner& runner) {
    runner.run("Plugin create returns positive ID", []() {
        Plugin plugin;
        plugin.name = "TestPlugin";
        plugin.version = "1.0.0";
        plugin.path = "/plugins/test.so";
        plugin.status = "active";

        int id = PluginRepository::create(plugin);
        assert(id >= 0);
    });

    runner.run("Plugin update changes status", []() {
        Plugin plugin;
        plugin.name = "TestPlugin";
        plugin.version = "1.0.0";
        plugin.path = "/plugins/test.so";
        plugin.status = "inactive";

        bool result = PluginRepository::update(1, plugin);
        assert(result == true);
    });
}

void testUpgradeRecordCRUD(TestRunner& runner) {
    runner.run("Upgrade record create returns positive ID", []() {
        UpgradeRecord record;
        record.target_version = "1.1.0";
        record.type = "full";
        record.status = "pending";
        record.rollback_version = "1.0.0";

        int id = UpgradeRecordRepository::create(record);
        assert(id >= 0);
    });

    runner.run("Upgrade record update works", []() {
        UpgradeRecord record;
        record.target_version = "1.1.0";
        record.type = "full";
        record.status = "success";
        record.rollback_version = "1.0.0";

        bool result = UpgradeRecordRepository::update(1, record);
        assert(result == true);
    });
}

void testTransactionManagement(TestRunner& runner) {
    runner.run("Transaction commits successfully", []() {
        Device device;
        device.name = "Tx Test Device";
        device.type = "camera";
        device.ip_address = "192.168.1.60";
        device.port = 554;

        // Manual transaction test
        bool result = DatabaseManager::instance().execute("BEGIN TRANSACTION");
        assert(result == true);

        int id = DeviceRepository::create(device);
        assert(id >= 0);

        result = DatabaseManager::instance().execute("COMMIT");
        assert(result == true);
    });
}

int main() {
    std::cout << "=== Database Layer Unit Tests ===" << std::endl;

    TestRunner runner;

    testDatabaseInitialization(runner);
    testDeviceCRUD(runner);
    testTaskCRUD(runner);
    testAlgorithmCRUD(runner);
    testAlarmRuleCRUD(runner);
    testAlarmEventCRUD(runner);
    testMonitoringMetricCRUD(runner);
    testConfigVersionCRUD(runner);
    testPluginCRUD(runner);
    testUpgradeRecordCRUD(runner);
    testTransactionManagement(runner);

    std::cout << std::endl;
    std::cout << "Results: " << runner.passed << " passed, " << runner.failed << " failed" << std::endl;

    // Cleanup
    DatabaseManager::instance().close();
    std::filesystem::remove(TEST_DB);

    return runner.failed > 0 ? 1 : 0;
}
