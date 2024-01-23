cmake_minimum_required(VERSION 3.10)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

project(PyModTestProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MØD
# -------------------------------------------------------------------------
find_package(mod REQUIRED)


# Boost.Python
# -------------------------------------------------------------------------
set(v 1.64.0)
foreach(PY 3 37 38 39 310 311 312 313 314 315 316 317 318 319)
    set(lib "python${PY}")
    find_package(Boost ${v} QUIET COMPONENTS ${lib})
    if(Boost_FOUND)
        find_package(Boost ${v} COMPONENTS ${lib})
        set(PYTHON_TARGET ${lib})
        break()
    endif()
endforeach()
if(NOT Boost_FOUND)
    find_package(Boost ${v} REQUIRED COMPONENTS python3)
    message(FATAL_ERROR "Could not find Boost.Python for Python 3. Tried 'python' wih suffixes 3, 37, 38, 39, and 310 to 319.")
endif()


# Python 3
# -------------------------------------------------------------------------
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)


# Artefacts
# -------------------------------------------------------------------------

add_library(awesome MODULE
        pyModule.cpp
        stuff.cpp)
set_target_properties(awesome PROPERTIES PREFIX "") # so it doesn't get the "lib" prefix
target_link_libraries(awesome PUBLIC mod::libmod Boost::${PYTHON_TARGET} Python3::Python)
target_compile_options(awesome PRIVATE -Wall -Wextra -pedantic
        -Wno-unused-parameter
        -Wno-comment
        -Wno-unused-local-typedefs)