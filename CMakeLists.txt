cmake_minimum_required(VERSION 3.0)
project(PingServices)

find_package(cppzmq)

add_executable(ping_services main.cpp)

target_link_libraries(ping_services cppzmq)
