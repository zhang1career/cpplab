include_directories(
        ${PROJECT_SOURCE_DIR}/../../lib/asio/include
        ${PROJECT_SOURCE_DIR}/../net_common/inc
        ${PROJECT_SOURCE_DIR}/inc
)

add_executable(${PROJECT_NAME} SimpleServer.cpp)