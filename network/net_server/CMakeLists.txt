cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

project(net_server)

add_subdirectory(
        src
        ${PROJECT_SOURCE_DIR}/../net_common/src
)