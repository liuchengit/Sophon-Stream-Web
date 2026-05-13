cmake_minimum_required(VERSION 3.15)
project(sophon-stream-plugins LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE PLUGIN_SOURCES "src/*.cpp")

add_library(sophon-stream-plugins STATIC ${PLUGIN_SOURCES})

target_link_libraries(sophon-stream-plugins
    PUBLIC
    ${CMAKE_DL_LIBS}
)
