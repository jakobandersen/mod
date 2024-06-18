set(workDir ${CMAKE_CURRENT_BINARY_DIR}/workDir/cmake_import_configure)
file(MAKE_DIRECTORY ${workDir})
add_test(NAME cmake_import_configure
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        ${CMAKE_CURRENT_SOURCE_DIR}/project
        WORKING_DIRECTORY ${workDir})
add_test(NAME cmake_import_build
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${workDir})
add_test(NAME cmake_import_run
        COMMAND ./doStuff
        WORKING_DIRECTORY ${workDir})
set_tests_properties(cmake_import_configure PROPERTIES FIXTURES_SETUP    cmake_import_build)
set_tests_properties(cmake_import_build     PROPERTIES FIXTURES_REQUIRED cmake_import_build)
set_tests_properties(cmake_import_build     PROPERTIES FIXTURES_SETUP    cmake_import_run)
set_tests_properties(cmake_import_run       PROPERTIES FIXTURES_REQUIRED cmake_import_run)