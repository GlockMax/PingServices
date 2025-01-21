cmake_minimum_required(VERSION 3.0)
project(PingServices)

find_package(cppzmq)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})


find_package(Boost COMPONENTS program_options REQUIRED)

add_executable(ping_services main.cpp)

target_include_directories(ping_services PRIVATE ${Boost_INCLUDE_DIR})

add_definitions("-g")

target_link_libraries(ping_services cppzmq)
target_link_libraries(ping_services ${JSONCPP_LIBRARIES})
target_link_libraries(ping_services ${Boost_PROGRAM_OPTIONS_LIBRARIES})

