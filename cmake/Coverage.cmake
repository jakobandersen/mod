# Module for enabling code coverage with gcov and recording information per test case.
# Based on:
# - https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake
# - https://swarminglogic.com/jotting/2014_05_lcov
#
# There are separate concerns:
#
# - Which libraries and executables should spit out coverage information.
# - Which commands/tests should generate coverage info as separate cases.
#
# Usage:
#
# 0. Perhaps redefine the default value of the "BUILD_COVERAGE" option.
#    It defaults to OFF, which means the module reduces to "do nothing".
# 1. include(Coverage)
# 2. for each target (libraries and (test) executables), use
#    target_add_coverage(targetName)
#    to add compile and link flags for generating information from them.
# 3. for each target that implements a test case which should generate separate
#    coverage information, use
#    add_coverage_case(targetName)
#    This adds a new target "targetName_cov" (the suffix can be configred with COVERAGE_TARGET_SUFFIX)
#    which will reset coverage info, run the test case, and save recorded coverage data.
#    The new target is added as a test with the label "coverage" (configurable through COVERAGE_TARGET_LABEL).
#    In ctest, use --print-labels, -L <regex>, and -LE <regex> to view and filter by label.
#    The target also has RUN_SERIAL as property.
#    Note: if you don't want per-test-case coverage, simply use this on a target that depends on all tests.
option(BUILD_COVERAGE "Enable code coverage." OFF)

if(NOT BUILD_COVERAGE)
    # define our functions as doing nothing
    function(target_add_coverage target)
    endfunction()
    function(add_coverage_case target)
    endfunction()
    return()
endif()

find_program(GCOV_PATH gcov)
find_program(LCOV_PATH NAMES lcov lcov.bat lcov.exe lcov.perl)
find_program(GENHTML_PATH NAMES genhtml genhtml.perl genhtml.bat)
message(STATUS "Coverage:")
if(GCOV_PATH)
    message(STATUS "  gcov found as: ${GCOV_PATH}")
else()
    message(FATAL_ERROR "  gcov not found.")
endif()
if(LCOV_PATH)
    message(STATUS "  lcov found as: ${LCOV_PATH}")
else()
    message(FATAL_ERROR "  lcov not found.")
endif()
if(GENHTML_PATH)
    message(STATUS "  genhtml found as: ${GENHTML_PATH}")
else()
    message(FATAL_ERROR "  genhtml not found.")
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    if("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 3)
        message(FATAL_ERROR "  Clang version must be 3.0.0 or greater.")
    endif()
elseif(NOT CMAKE_COMPILER_IS_GNUCXX)
    message(FATAL_ERROR "  Compiler must be Clang >= 3 nor GNU GCC.")
endif()

set(COVERAGE_COMPILE_FLAGS
        -g -O0 --coverage
        CACHE STRING "Flags used by the C/C++ compiler during coverage builds."
        FORCE)
set(COVERAGE_LINK_FLAGS
        --coverage
        CACHE STRING "Flags used for linking during coverage builds."
        FORCE)
set(COVERAGE_TARGET_SUFFIX
        _cov
        CACHE STRING "Label added to test case targets for filtering in ctest."
        FORCE)
set(COVERAGE_TARGET_LABEL
        coverage
        CACHE STRING "Suffix added to test case targets for recording the coverage information."
        FORCE)
mark_as_advanced(
        COVERAGE_COMPILE_FLAGS
        COVERAGE_LINK_FLAGS
        COVERAGE_TARGET_SUFFIX
        COVERAGE_TARGET_LABEL)

set(COVERAGE_BUILD ${CMAKE_CURRENT_LIST_DIR}/CoverageBuild.sh)

# The target to run all coverage test cases.
add_custom_target(coverage_collect)
# The target to build the coverage report. This does explicitly not depend on coverage_collect
# in order to allow partial results.
add_custom_target(coverage_build
        COMMAND SRC=${CMAKE_SOURCE_DIR} LCOV=${LCOV_PATH} GENHTML=${GENHTML_PATH} PROJECT_NAME=${PROJECT_NAME} PROJECT_VERSION=${PROJECT_VERSION} ${COVERAGE_BUILD}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

# Shorthand for adding compile and link flags to a target.
function(target_add_coverage target)
    get_target_property(type ${target} TYPE)
    if(${type} STREQUAL "INTERFACE_LIBRARY")
        set(type INTERFACE)
    else()
        set(type PRIVATE)
    endif()
    target_compile_options(${target} ${type} ${COVERAGE_COMPILE_FLAGS})
    target_link_libraries(${target} ${type} ${COVERAGE_LINK_FLAGS})
endfunction()

# Set up a target for coverage recording.
function(add_coverage_case target)
    set(covTarget "${target}${COVERAGE_TARGET_SUFFIX}")
    add_custom_target(${covTarget}
            #DEPENDS ${target}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/cov
            COMMAND ${LCOV_PATH} -z -d ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_CTEST_COMMAND} -R "^${target}\$"
            COMMAND ${LCOV_PATH} -c -d ${CMAKE_BINARY_DIR} -t ${target} -o ${CMAKE_BINARY_DIR}/cov/${target}.all.cov
    )
    add_dependencies(coverage_collect ${covTarget})
endfunction()