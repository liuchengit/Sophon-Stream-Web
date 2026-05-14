cmake_minimum_required(VERSION 3.15)
project(sophon-stream-engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE ENGINE_SOURCES "src/*.cpp")

add_library(sophon-stream-engine STATIC ${ENGINE_SOURCES})

target_link_libraries(sophon-stream-engine
    PUBLIC
    nlohmann_json::nlohmann_json
)

# Install headers
install(DIRECTORY include/ DESTINATION include/sophon-stream-engine)
