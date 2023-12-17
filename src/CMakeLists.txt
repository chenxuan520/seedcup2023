# cmake version
cmake_minimum_required(VERSION 3.16.3)
project(seedcup2023)

set(PROJECT_ROOT_PATH .)
# set(CMAKE_CURRENT_BINARY_DIR ${PROJECT_ROOT_PATH}/build)

add_subdirectory(${PROJECT_SOURCE_DIR}/server)
