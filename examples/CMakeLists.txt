if(NOT BUILD_EXAMPLES)
    return()
endif()

# libmod_cmake
# =============================================================================
set(workDir ${CMAKE_CURRENT_BINARY_DIR}/workDir/libmod_cmake)
file(MAKE_DIRECTORY ${workDir})
add_test(NAME example__libmod_cmake_configure
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        ${CMAKE_CURRENT_SOURCE_DIR}/libmod_cmake
        WORKING_DIRECTORY ${workDir})
add_test(NAME example__libmod_cmake_build
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${workDir})
add_test(NAME example__libmod_cmake_run
        COMMAND ./doStuff
        WORKING_DIRECTORY ${workDir})
set_tests_properties(example__libmod_cmake_configure PROPERTIES FIXTURES_SETUP    example_libmod_cmake_build)
set_tests_properties(example__libmod_cmake_build     PROPERTIES FIXTURES_REQUIRED example_libmod_cmake_build)
set_tests_properties(example__libmod_cmake_build     PROPERTIES FIXTURES_SETUP    example_libmod_cmake_run)
set_tests_properties(example__libmod_cmake_run       PROPERTIES FIXTURES_REQUIRED example_libmod_cmake_run)

# pymod_extension
# =============================================================================
if(BUILD_PY_MOD)
    set(workDir ${CMAKE_CURRENT_BINARY_DIR}/workDir/pymod_extension)
    file(MAKE_DIRECTORY ${workDir})
    add_test(NAME example__pymod_extension_configure
            COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            ${CMAKE_CURRENT_SOURCE_DIR}/pymod_extension
            WORKING_DIRECTORY ${workDir})
    add_test(NAME example__pymod_extension_build
            COMMAND ${CMAKE_COMMAND} --build .
            WORKING_DIRECTORY ${workDir})
    add_test(NAME example__pymod_extension_run
            COMMAND ${CMAKE_INSTALL_FULL_BINDIR}/mod -f ${CMAKE_CURRENT_SOURCE_DIR}/pymod_extension/test.py
            WORKING_DIRECTORY ${workDir})
    set_tests_properties(example__pymod_extension_configure PROPERTIES FIXTURES_SETUP    example_pymod_extension_build)
    set_tests_properties(example__pymod_extension_build     PROPERTIES FIXTURES_REQUIRED example_pymod_extension_build)
    set_tests_properties(example__pymod_extension_build     PROPERTIES FIXTURES_SETUP    example_pymod_extension_run)
    set_tests_properties(example__pymod_extension_run       PROPERTIES FIXTURES_REQUIRED example_pymod_extension_run)
endif()

# py examples
# =============================================================================
if(BUILD_PY_MOD)
    foreach(fileName ${mod_EXAMPLES_PY_FILES})
        string(REPLACE "/" "__" testName "example__${fileName}")
        make_py_test(${fileName} ${testName} "")
    endforeach()
endif()