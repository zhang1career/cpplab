include_directories(
        ${PROJECT_SOURCE_DIR}/../../lib/asio/include
        ${PROJECT_SOURCE_DIR}/../net_common/inc
)

add_executable(${PROJECT_NAME} SimpleServer.cpp)