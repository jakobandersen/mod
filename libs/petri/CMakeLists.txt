###########################################################################
# Targets and Artefacts
###########################################################################

add_library(petri STATIC
        ${mod_petri_INCLUDE_FILES}
        ${mod_petri_SRC_FILES})
set_property(TARGET petri PROPERTY POSITION_INDEPENDENT_CODE ON)
add_library(Petri::petri ALIAS petri)
target_include_directories(petri
        PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(petri
        PUBLIC Boost::boost
        PRIVATE JLA::boost
        )
target_compile_options(petri PRIVATE -Wall -Wextra -pedantic
        -Wno-unused-parameter
        -Wno-sign-compare
        -Wno-mismatched-tags)
set_target_properties(petri PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
target_compile_definitions(petri PRIVATE PETRI_SOURCE)

handle_sanitizers(petri)
target_add_coverage(petri)

install(TARGETS petri
        EXPORT PROJECT_exports
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT PETRI_lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT PETRI_lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT PETRI_lib)
install(DIRECTORY include/petri
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT PETRI_dev
        FILES_MATCHING PATTERN "*.hpp")

###########################################################################
# Tests
###########################################################################

if(BUILD_TESTING)
    if(NOT TARGET tests)
        add_custom_target(tests)
    endif()
    add_dependencies(tests ${mod_petri_TEST_CPP_FILES})

    if("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xAppleClang")
        set(sanFlags -g -fsanitize=undefined -fsanitize=address)
    else()
        set(sanFlags -g -fsanitize=undefined -fsanitize=address -fsanitize=leak)
    endif()
    foreach(testName ${mod_petri_TEST_CPP_FILES})
        add_executable(${testName} EXCLUDE_FROM_ALL test/${testName}.cpp)
        target_link_libraries(${testName} PRIVATE Petri::petri)
        handle_sanitizers(${testName})
        target_add_coverage(${testName})
        add_test(${testName} ${testName})
        add_coverage_case(${testName})
    endforeach()
endif()