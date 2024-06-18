cmake_minimum_required(VERSION 3.15)

project(CMakeTestProject CXX)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_package(mod REQUIRED)

add_executable(doStuff main.cpp)
target_link_libraries(doStuff mod::libmod)