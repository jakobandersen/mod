set(workDir ${CMAKE_CURRENT_BINARY_DIR}/workDir/cmake_add_subdirectory_configure)
file(MAKE_DIRECTORY ${workDir})
add_test(NAME cmake_add_subdirectory_configure
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}"
        -Dmod_DIR=${PROJECT_SOURCE_DIR}
        -DBOOST_ROOT=${BOOST_ROOT}
        -DWITH_GUROBI=${WITH_GUROBI}
        -DGUROBI_DIR=${GUROBI_DIR}
        -DWITH_CPLEX=${WITH_CPLEX}
        -DCPLEX_DIR=${CPLEX_DIR}
        -DWITH_OPENBABEL=${WITH_OPENBABEL}
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/project
        WORKING_DIRECTORY ${workDir})
add_test(NAME cmake_add_subdirectory_build
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${workDir})
set_tests_properties(cmake_add_subdirectory_configure PROPERTIES
        FIXTURES_SETUP cmake_add_subdirectory)
set_tests_properties(cmake_add_subdirectory_build PROPERTIES
        FIXTURES_REQUIRED cmake_add_subdirectory)
