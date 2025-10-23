###########################################################################
# Targets and Artefacts
###########################################################################

# pymodutils
# -------------------------------------------------------------------------
add_library(pymodutils INTERFACE)
# TODO: when CMake 3.19 is required, add the sources
#       https://cmake.org/cmake/help/latest/command/add_library.html#interface-libraries
#        ${mod_pymodutils_INCLUDE_FILES}
#        ${mod_pymodtils_SRC_FILES})
add_library(mod::pymodutils ALIAS pymodutils)
target_include_directories(pymodutils INTERFACE
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_link_libraries(pymodutils INTERFACE
        mod::libmod
        Boost::${PYTHON_TARGET}
        Python3::Python)

install(TARGETS pymodutils
        EXPORT PROJECT_exports
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/mod COMPONENT pymodutils_lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/mod COMPONENT pymodutils_lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}     COMPONENT pymodutils_lib)
install(DIRECTORY src/mod
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT pymodutils_dev
        FILES_MATCHING PATTERN "*.hpp")