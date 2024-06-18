cmake_minimum_required(VERSION 3.26)
project(netlibcmake)
set(CMAKE_CXX_STANDARD 23)

set(NETLIB_PATH netlib)
add_library(netlib
        ${NETLIB_PATH}/codes.cpp
        ${NETLIB_PATH}/utils.cpp
        ${NETLIB_PATH}/packet.cpp
        ${NETLIB_PATH}/stream.cpp
        ${NETLIB_PATH}/client.cpp
        ${NETLIB_PATH}/server.cpp
        ${NETLIB_PATH}/broadcaster.cpp
)
target_include_directories(netlib INTERFACE ${NETLIB_PATH})
target_link_libraries(netlib ws2_32)#link winsock library

add_executable(client_sample client_sample.cpp)
add_executable(server_sample server_sample.cpp)
target_link_libraries(client_sample netlib)
target_link_libraries(server_sample netlib)