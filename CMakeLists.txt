cmake_minimum_required(VERSION 3.20)
project(ScoreHiveCluster VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_SOURCES
    source/main.cpp
    source/server/server.cpp
    source/system/environment.cpp
    source/domain/answers.cpp
    source/domain/coordinator.cpp
    source/domain/evaluator.cpp
)

set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -Wpedantic -Werror -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -Wpedantic -Werror -O0 -g")

find_package(MPI REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)
target_link_libraries(${PROJECT_NAME} PRIVATE MPI::MPI_CXX spdlog::spdlog nlohmann_json::nlohmann_json)
