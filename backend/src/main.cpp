#include <drogon/drogon.h>
#include <iostream>
#include <filesystem>
#include "db/database_manager.h"
#include "auth/auth_service.h"
#include "stream_engine.h"
#include "middleware/auth_middleware.h"

using namespace drogon;
using namespace sophon::web::middleware;

int main() {
    std::cout << "Starting Sophon-Stream Web Management System..." << std::endl;

    // Ensure required directories exist
    std::filesystem::create_directories("logs");
    std::filesystem::create_directories("uploads");
    std::filesystem::create_directories("data");
    std::filesystem::create_directories("engine/configs");

    // Initialize database
    if (!sophon::web::db::DatabaseManager::instance().initialize("data/sophon-web.db")) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }

    // Initialize authentication
    if (!sophon::web::auth::AuthService::instance().initialize()) {
        std::cerr << "Failed to initialize auth service" << std::endl;
        return 1;
    }

    // Initialize sophon-stream engine
    if (!sophon::stream::StreamEngine::instance().initialize("engine/configs/default.json")) {
        std::cerr << "Warning: Failed to initialize sophon-stream engine (running without hardware)" << std::endl;
    }

    // Register middlewares
    drogon::app().registerMiddleware(std::make_shared<AuthMiddleware>());
    drogon::app().registerMiddleware(std::make_shared<RBACMiddleware>());

    // Load configuration
    drogon::app().loadConfigFile("config.json");

    std::cout << "Sophon-Stream Web Management System started successfully." << std::endl;
    std::cout << "API: http://localhost:8080" << std::endl;
    std::cout << "Default admin: admin / admin123" << std::endl;

    // Start the application
    drogon::app().run();

    // Cleanup
    sophon::stream::StreamEngine::instance().shutdown();
    sophon::web::db::DatabaseManager::instance().close();

    return 0;
}
