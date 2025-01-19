cmake_minimum_required(VERSION 3.0)
project(PingServices)

find_package(cppzmq)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

add_executable(ping_services main.cpp)

add_definitions("-g")

target_link_libraries(ping_services cppzmq)
target_link_libraries(ping_services ${JSONCPP_LIBRARIES})
