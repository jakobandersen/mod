if(NOT BUILD_DOC)
    return()
endif()

set(output ${CMAKE_CURRENT_BINARY_DIR}/html)
set(doctrees ${CMAKE_CURRENT_BINARY_DIR}/doctrees)
add_custom_command(
        OUTPUT ${output}
        COMMAND ${SPHINX} ${CMAKE_CURRENT_SOURCE_DIR}/source -n -b html ${output} -d ${doctrees}
        DEPENDS ${mod_DOC_FILES})
if(TARGET doc)
    add_dependencies(doc ${output})
else()
    add_custom_target(doc ALL DEPENDS ${output})
endif()
set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${output} ${doctrees})
install(DIRECTORY ${output}
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        COMPONENT MOD_doc)

# doc examples
# =============================================================================
if(BUILD_PY_MOD)
    foreach(fileName ${mod_DOC_PY_FILES})
        string(REPLACE "/" "__" testName "doc__${fileName}")
        make_py_test(source/${fileName} ${testName} "")
    endforeach()
endif()