cmake_minimum_required(VERSION 3.15)
project(sophon-stream-web-backend VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Drogon
find_package(Drogon CONFIG REQUIRED)

# Find other dependencies
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.cc"
)

# Add executable
add_executable(sophon-stream-web ${SOURCES})

# Link libraries
target_link_libraries(sophon-stream-web
    PRIVATE
    Drogon::Drogon
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
)

# Install
install(TARGETS sophon-stream-web DESTINATION bin)

# Copy config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.json
    ${CMAKE_CURRENT_BINARY_DIR}/config.json
    COPYONLY
)

# Create www directory for frontend build output
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/www)
