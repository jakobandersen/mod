if(NOT BUILD_TESTING)
    return()
endif()

if(NOT TARGET tests)
    add_custom_target(tests)
endif()

if("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xAppleClang")
    set(sanFlags -g -fsanitize=undefined -fsanitize=address)
else()
    set(sanFlags -g -fsanitize=undefined -fsanitize=address -fsanitize=leak)
endif()
foreach(fileName ${mod_TEST_CPP_FILES})
    string(REPLACE "/" "__" testName "${fileName}")
    set(workDir ${CMAKE_CURRENT_BINARY_DIR}/workDir/${testName})
    file(MAKE_DIRECTORY ${workDir})
    add_executable(${testName} EXCLUDE_FROM_ALL ${fileName}.cpp)
    target_compile_options(${testName} PRIVATE -Wall -Wextra -pedantic -Werror
            -Wno-comment)
    target_link_libraries(${testName} PRIVATE mod::libmod)
    handle_sanitizers(${testName})
    add_dependencies(tests ${testName})
    target_add_coverage(${testName})
    add_test(NAME ${testName}
            COMMAND ${testName}
            WORKING_DIRECTORY ${workDir})
    add_coverage_case(${testName})
endforeach()

if(BUILD_PY_MOD)
    foreach(fileName ${mod_TEST_PY_FILES})
        string(REPLACE "/" "__" testName "${fileName}")
        make_py_test(${fileName} ${testName} "")
    endforeach()

    add_test(NAME pymod-module-direct
            COMMAND ${CMAKE_INSTALL_FULL_BINDIR}/mod --nopost -e "assert not mod._redirected")
    add_test(NAME pymod-module-redirected
            COMMAND ${Python3_EXECUTABLE} -c "import mod\nassert mod._redirected")
endif()

add_subdirectory(cmake_add_subdirectory)
add_subdirectory(cmake_import)