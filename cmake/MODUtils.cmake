

function(make_py_test fileName testName extraEnv)
    set(workDir ${CMAKE_CURRENT_BINARY_DIR}/workDir/${testName})
    file(MAKE_DIRECTORY ${workDir})
    add_test(NAME ${testName}
            COMMAND ${CMAKE_INSTALL_FULL_BINDIR}/mod -f ${CMAKE_CURRENT_LIST_DIR}/${fileName}.py
            WORKING_DIRECTORY ${workDir})
    set_tests_properties(${testName} PROPERTIES
            ENVIRONMENT "MOD_NUM_POST_THREADS=1;MOD_NO_DEPRECATED=1;PYTHONWARNINGS=error${extraEnv}")
    add_coverage_case(${testName})
endfunction()

function(handle_sanitizers target)
    if(BUILD_WITH_SANITIZERS AND NOT BUILD_COVERAGE)
        if("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xAppleClang")
            set(sanFlags -g -fsanitize=undefined,address)
        else()
            set(sanFlags -g -fsanitize=undefined,address,leak)
        endif()
        target_compile_options(${target} PRIVATE ${sanFlags})
        target_link_libraries(${target} PRIVATE ${sanFlags})
    endif()
endfunction()