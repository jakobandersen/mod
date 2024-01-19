###########################################################################
# Targets and Artefacts
###########################################################################

add_library(gml STATIC
        ${mod_gml_INCLUDE_FILES}
        ${mod_gml_SRC_FILES})
add_library(GML::gml ALIAS gml)
target_include_directories(gml
        PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_options(gml PRIVATE -Wall -Wextra -pedantic
        -Wno-unused-parameter
        -Wno-parentheses)
target_link_libraries(gml PUBLIC Boost::boost)
set_target_properties(gml PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
target_compile_definitions(gml PRIVATE GML_SOURCE)

handle_sanitizers(gml)
target_add_coverage(gml)

install(TARGETS gml
        EXPORT PROJECT_exports
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT GML_lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT GML_lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT GML_lib)
install(DIRECTORY include/gml
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT GML_dev
        FILES_MATCHING PATTERN "*.hpp")


###########################################################################
# Tests
###########################################################################

if(BUILD_TESTING)
    if(NOT TARGET tests)
        add_custom_target(tests)
    endif()
    add_dependencies(tests ${mod_gml_TEST_CPP_FILES})

    foreach(testName ${mod_gml_TEST_CPP_FILES})
        add_executable(${testName} EXCLUDE_FROM_ALL test/${testName}.cpp)
        target_compile_options(${testName} PRIVATE -Wall -Wextra -pedantic -Werror
                -Wno-unused-parameter
                -Wno-parentheses)
        target_link_libraries(${testName} PRIVATE GML::gml)
        handle_sanitizers(${testName})
        target_add_coverage(${testName})
        add_test(${testName} ${testName})
        add_coverage_case(${testName})
    endforeach()
endif()