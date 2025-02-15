cmake_minimum_required(VERSION 3.15)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

project(PyModTestProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MØD
# -------------------------------------------------------------------------
find_package(mod REQUIRED)

# Artefacts
# -------------------------------------------------------------------------
add_library(awesome MODULE
        pyModule.cpp
        stuff.cpp)
set_target_properties(awesome PROPERTIES PREFIX "") # so it doesn't get the "lib" prefix
target_link_libraries(awesome PUBLIC mod::pymodutils)
target_compile_options(awesome PRIVATE -Wall -Wextra -pedantic
        -Wno-unused-parameter
        -Wno-comment
        -Wno-unused-local-typedefs)