# BuildConfig.h
# -------------------------------------------------------------------------
configure_file(BuildConfig.hpp.in include/mod/BuildConfig.hpp @ONLY)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/mod
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT mod_dev)


# libmod
# -------------------------------------------------------------------------
add_library(libmod SHARED
        ${mod_libmod_INCLUDE_FILES}
        ${mod_libmod_SRC_FILES})
add_library(mod::libmod ALIAS libmod)
handle_sanitizers(libmod)
target_add_coverage(libmod)
target_include_directories(libmod
        PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
if(ENABLE_DEP_SYMBOL_HIDING)
    target_link_libraries(libmod PRIVATE
            $<$<CXX_COMPILER_ID:GNU>:-Wl,--exclude-libs,ALL>
            $<$<CXX_COMPILER_ID:Clang>:-Wl,--exclude-libs,ALL>
            #$<$<CXX_COMPILER_ID:AppleClang>:-Wl,--exclude-libs,ALL> # TODO: is a an equivalent?
            )
endif()
target_link_libraries(libmod
        PUBLIC
            JLA::boost
        PRIVATE
            GML::gml Threads::Threads
            Boost::graph Boost::iostreams
            GraphCanon::graph_canon
            nlohmann_json::nlohmann_json
            nlohmann_json_schema_validator
            $<$<BOOL:${WITH_OPENBABEL}>:OpenBabel::OpenBabel>
        )
# Boost::graph needs Boost::regex which needs the RPATH of libmod to have Boost in it.
# But because all Boost libs are linked privately the path is not exposed.
# So if someone links against libmod, but no Boost lib, then the linking will fail.
# Perhaps fixed in https://gitlab.kitware.com/cmake/cmake/-/merge_requests/3642
# TODO: remove the next line with some higher CMAKE version is required?
target_link_libraries(libmod PUBLIC Boost::iostreams)
target_link_libraries(libmod PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wl,--no-undefined>
        $<$<CXX_COMPILER_ID:Clang>:-Wl,--no-undefined>
        #$<$<CXX_COMPILER_ID:AppleClang>:-Wl,--no-undefined> # TODO: is there an equivalent?
        )
# https://stackoverflow.com/questions/52018092/how-to-set-rpath-and-runpath-with-gcc-ld
target_link_libraries(libmod PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wl,--disable-new-dtags>
        $<$<CXX_COMPILER_ID:Clang>:-Wl,--disable-new-dtags>
        )
target_compile_options(libmod PRIVATE -Wall -Wextra -pedantic
        -Wno-unused-parameter
        -Wno-comment
        -Wno-sign-compare
        -Wno-unused-local-typedefs
        # gives false positive for boost::optional
        $<$<CXX_COMPILER_ID:GNU>:-Wno-error=maybe-uninitialized>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-unknown-pragmas> # for PetriNet.cpp
        # TODO: remove when C++20/P1825R0 is available
        $<$<CXX_COMPILER_ID:GNU>:-Wno-redundant-move>
        )
target_compile_options(libmod PUBLIC
        -DBOOST_ALLOW_DEPRECATED_HEADERS # until Boost stops using include/boost/detail/iterator.hpp
        $<$<CXX_COMPILER_ID:Clang>:-Wno-mismatched-tags>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wno-mismatched-tags>)
set_target_properties(libmod PROPERTIES OUTPUT_NAME "mod")
set_target_properties(libmod PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
if(ENABLE_SYMBOL_HIDING)
    set_target_properties(libmod PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            # The inlines in Open Babel needs to be visible for the plugins to work.
            # GCC does fine, but not Clang, hence the generator expression.
            $<$<CXX_COMPILER_ID:GNU>VISIBILITY_INLINES_HIDDEN ON>
            )
endif()
target_compile_definitions(libmod PRIVATE MOD_SOURCE)
if(ENABLE_IPO)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set_property(TARGET libmod PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(FATAL_ERROR "Interprocedural optimization was requested ('ENABLE_IPO=on'), but it is not supported: ${output}")
    endif()
endif()

install(TARGETS libmod
        EXPORT PROJECT_exports
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT mod_lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT mod_lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT mod_lib)
install(DIRECTORY src/mod
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT mod_dev
        FILES_MATCHING PATTERN "*.hpp")


# CMake Package Support
# -------------------------------------------------------------------------
include(CMakePackageConfigHelpers)
set(exportDir ${CMAKE_CURRENT_BINARY_DIR}/lib/cmake/${PROJECT_NAME})
configure_file(modConfig.cmake ${exportDir}/modConfig.cmake @ONLY)
foreach(f ${libmod_config_find_files})
    configure_file(${PROJECT_SOURCE_DIR}/cmake/${f} ${exportDir}/${f} COPYONLY)
endforeach()
write_basic_package_version_file(
        ${exportDir}/${PROJECT_NAME}Config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
install(EXPORT PROJECT_exports
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME}
        FILE ${PROJECT_NAME}Targets.cmake)
install(DIRECTORY ${exportDir}
        DESTINATION lib/cmake
        COMPONENT mod_dev)
