#include <drogon/drogon.h>
#include <iostream>
#include <filesystem>

int main() {
    std::cout << "Starting Sophon-Stream Web Management System..." << std::endl;

    // Ensure required directories exist
    std::filesystem::create_directories("logs");
    std::filesystem::create_directories("uploads");
    std::filesystem::create_directories("data");

    // Load configuration
    drogon::app().loadConfigFile("config.json");

    // Register controllers (auto-discovered by Drogon)

    // Start the application
    drogon::app().run();

    return 0;
}
