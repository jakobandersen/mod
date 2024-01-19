if(NOT BUILD_PY_MOD)
    return()
endif()

###########################################################################
# Targets and Artefacts
###########################################################################

# libpymod
# -------------------------------------------------------------------------
add_library(pymod MODULE
        ${mod_pymod_INCLUDE_FILES}
        ${mod_pymod_SRC_FILES})
add_library(mod::pymod ALIAS pymod)
target_include_directories(pymod
        PUBLIC
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_link_libraries(pymod PUBLIC mod::pymodutils)
target_link_libraries(pymod PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wl,--no-undefined>
        $<$<CXX_COMPILER_ID:Clang>:-Wl,--no-undefined>
        #$<$<CXX_COMPILER_ID:AppleClang>:-Wl,--no-undefined> # TODO: is there an equivalent?
        )
set_target_properties(pymod PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}")
target_compile_options(pymod PRIVATE -Wall -Wextra -pedantic
        -Wno-unused-parameter
        -Wno-comment)
set_target_properties(pymod PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON)
target_compile_definitions(pymod PRIVATE PYMOD_SOURCE)
if(ENABLE_IPO)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set_property(TARGET pymod PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(FATAL_ERROR "Interprocedural optimization was requested ('ENABLE_IPO=on'), but it is not supported: ${output}")
    endif()
endif()

install(TARGETS pymod
        EXPORT PROJECT_exports
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/mod COMPONENT pymod_lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/mod COMPONENT pymod_lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}     COMPONENT pymod_lib)
install(DIRECTORY src/mod
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT pymod_dev
        FILES_MATCHING PATTERN "*.hpp")

# the plugin file
set(exportDir ${CMAKE_CURRENT_BINARY_DIR}/share/mod/plugins)
configure_file(share/mod/plugins/00_mod.in ${exportDir}/00_mod.in @ONLY)
install(FILES ${exportDir}/00_mod.in
        DESTINATION ${CMAKE_INSTALL_DATADIR}/mod/plugins/
        COMPONENT pymod_run)

# module files
install(DIRECTORY lib/mod
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT pymod_run
        FILES_MATCHING PATTERN "*.py" PATTERN "*.pyi")

# redirector
# -------------------------------------------------------------------------

set(exportDir ${CMAKE_CURRENT_BINARY_DIR}/redirector)
file(MAKE_DIRECTORY ${exportDir}/mod)
# TODO: should be file(COPY_FILE redirector/pyproject.toml ${exportDir}/pyproject.toml) when CMake 3.21 is required
configure_file(redirector/pyproject.toml ${exportDir}/pyproject.toml @ONLY)
configure_file(redirector/setup.cfg.in ${exportDir}/setup.cfg @ONLY)
configure_file(redirector/mod/__init__.py.in ${exportDir}/mod/__init__.py @ONLY)
if(${BUILD_PY_MOD_PIP})
    install(CODE "execute_process(COMMAND ${Python3_EXECUTABLE} -m pip install . WORKING_DIRECTORY ${exportDir})"
            COMPONENT pymod_run)
endif()

# mod
# -------------------------------------------------------------------------
set(exportDir ${CMAKE_CURRENT_BINARY_DIR}/bin)
configure_file(bin/mod.in ${exportDir}/mod @ONLY)
install(PROGRAMS ${exportDir}/mod
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT pymod_run)


# Valgrind suppression files
# -------------------------------------------------------------------------
install(FILES
            share/mod/obabel.supp
            share/mod/python.supp
            share/mod/extra.supp
        DESTINATION ${CMAKE_INSTALL_DATADIR}/mod
        COMPONENT pymod_run)