cmake_minimum_required(VERSION 3.15)
project(sophon-stream-web VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# Add subdirectories
add_subdirectory(engine)
add_subdirectory(backend)
add_subdirectory(plugins)

# Installation
install(DIRECTORY scripts/ DESTINATION bin)
