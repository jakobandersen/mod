cmake_minimum_required(VERSION 3.15)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(FindPythonModule)
include(MODUtils)

###########################################################################
# Package Properties
###########################################################################
file(READ "VERSION" PROJECT_VERSION)
string(STRIP "${PROJECT_VERSION}" PROJECT_VERSION) # remove the newline

project(mod VERSION ${PROJECT_VERSION})
set(mod_VERSION ${PROJECT_VERSION} CACHE INTERNAL "" FORCE)
set(PNAME_FILE mod) # how the project name should be written in file names
set(mod_AUTHOR "Jakob Lykke Andersen")
set(mod_AUTHOR_EMAIL "jlandersen@imada.sdu.dk")
set(CPACK_PACKAGE_CONTACT "${mod_AUTHOR} <${mod_AUTHOR_EMAIL}>")


set(CMAKE_CXX_FLAGS_OPTDEBUG "-g -O3")
set(CMAKE_C_FLAGS_OPTDEBUG "-g -O3")
set(CMAKE_EXE_LINKER_FLAGS_OPTDEBUG "")
set(CMAKE_SHARED_LINKER_FLAGS_OPTDEBUG "")
mark_as_advanced(CMAKE_CXX_FLAGS_OPTDEBUG CMAKE_C_FLAGS_OPTDEBUG CMAKE_EXE_LINKER_FLAGS_OPTDEBUG CMAKE_SHARED_LINKER_FLAGS_OPTDEBUG)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
            "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel OptDebug."
            FORCE)
endif()


###########################################################################
# Options
###########################################################################
option(BUILD_DOC "Enable documentation building." OFF)
option(BUILD_POST_MOD "Enable building of the post processor." ON)
option(BUILD_POST_MOD_FMT "Enable building of the post processor Latex format files." ON)
option(BUILD_PY_MOD "Enable building of the Python bindings." ON)
option(BUILD_PY_MOD_PIP "If BUILD_PY_MOD, then install the bindings also with pip in the default location." ON)
option(BUILD_EPIM "Enable building of the EpiM extension (requires PyMØD)." ON)

if(BUILD_EPIM AND NOT BUILD_PY_MOD)
    message(FATAL_ERROR
            "Epim requested but PyMØD is disabled.\n"
            "Either set BUILD_EPIM=off or set BUILD_PY_MOD=on")
endif()

option(BUILD_TESTING "Enable test building." OFF)
option(BUILD_WITH_SANITIZERS "Compile libraries and tests with sanitizers." OFF)
option(BUILD_EXAMPLES "Enable example as tests." ON)
option(BUILD_COVERAGE "Enable code coverage." OFF)

option(ENABLE_SYMBOL_HIDING "Hide internal symbols in the library." ON)
option(ENABLE_DEP_SYMBOL_HIDING "Hide symbols provided by library dependencies." ON)
option(ENABLE_IPO "Using link-time optimization." ON)

option(USE_NESTED_GRAPH_CANON "Use the GraphCanon version in external/graph_canon." ON)
option(USE_NESTED_NLOHMANN_JSON "Use the nlohmann/json version in external/nlohmann_json." ON)
option(USE_NESTED_JSON_SCHEMA "Use the pboettch/json-schema-validator version in external/json_schema." ON)

option(WITH_OPENBABEL "Whether features depending on Open Babel are enabled or not." ON)


get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(BUILD_DOC 0)
    set(BUILD_EXAMPLES 0)
    set(BUILD_TESTING 0)
    set(BUILD_COVERAGE 0)
else()
    set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
    include(CheckIPOSupported)
    if(ENABLE_IPO)
        check_ipo_supported(RESULT result OUTPUT output)
        if(result)
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        else()
            message(FATAL_ERROR "Interprocedural optimization was requested ('ENABLE_IPO=on'), but it is not supported: ${output}")
        endif()
    else()
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
    endif()
endif()
if(NOT BUILD_TESTING)
    set(BUILD_EXAMPLES OFF)
endif()

enable_testing()  # should be included here to add the targets in the top-level folder

###########################################################################
# Individual checks
###########################################################################
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(libmod_config_dependencies "")
set(libmod_config_find_files "")


# Boost
# -------------------------------------------------------------------------
set(v 1.76.0)
if(BUILD_PY_MOD)
    foreach(PY 3 37 38 39 310 311 312 313 314 315 316 317 318 319)
        set(lib "python${PY}")
        find_package(Boost ${v} QUIET COMPONENTS ${lib})
        if(Boost_FOUND)
            find_package(Boost ${v} COMPONENTS ${lib})
            set(PYTHON_TARGET ${lib})
            string(APPEND libmod_config_dependencies "find_dependency(Boost ${v} COMPONENTS ${lib})\n")
            break()
        endif()
    endforeach()
    if(NOT Boost_FOUND)
        find_package(Boost ${v} COMPONENTS python3)
        message(FATAL_ERROR "Could not find Boost.Python for Python 3. Tried 'python' wih suffixes 3, 37, 38, 39, and 310 to 319.")
    endif()
endif()
if(BUILD_TESTING)
    find_package(Boost ${v} REQUIRED COMPONENTS graph iostreams unit_test_framework)
else()
    find_package(Boost ${v} REQUIRED COMPONENTS graph iostreams)
endif()
string(APPEND libmod_config_dependencies "find_dependency(Boost ${v} COMPONENTS graph iostreams)\n")
message(STATUS "(Boost include dir is ${Boost_INCLUDE_DIRS})")


# GraphCanon
# -------------------------------------------------------------------------
set(v 0.5)
if(USE_NESTED_GRAPH_CANON)
    message(STATUS "GraphCanon: using external/graph_canon")
    add_subdirectory(external/graph_canon)
    if(GraphCanon_VERSION VERSION_LESS v)
        message(FATAL_ERROR "Nested GraphCanon version not new enough: is ${GraphCanon_VERSION}, needs ${v}."
                " Try again after 'git submodule update --init --recursive; ./bootstrap.sh'."
                " Otherwise please report as bug.")
    endif()
else()
    find_package(GraphCanon ${v} REQUIRED)
    string(APPEND libmod_config_dependencies "find_dependency(GraphCanon ${v})\n")
    message(STATUS "GraphCanon version: ${GraphCanon_VERSION}")
endif()


# JSON and JSON Schema
# -------------------------------------------------------------------------
set(v 3.7.3)
if(USE_NESTED_NLOHMANN_JSON)
    message(STATUS "nlohmann/json: using external/nlohmann_json")
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    add_subdirectory(external/nlohmann_json)
else()
    find_package(nlohmann_json ${v} REQUIRED)
endif()

# (must be after nlohmann/json to satisfy the dependency)
set(v 2.1.1)
if(USE_NESTED_JSON_SCHEMA)
    message(STATUS "pboettch/json-schema-validator: using external/json_schema")
    add_subdirectory(external/json_schema)
else()
    find_package(nlohmann_json_schema_validator ${v} REQUIRED)
endif()
set_target_properties(nlohmann_json_schema_validator PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
# TODO: https://github.com/pboettch/json-schema-validator/issues/108
target_compile_options(nlohmann_json_schema_validator PRIVATE -Wno-error=switch)


# Graphviz
# -------------------------------------------------------------------------
if(BUILD_POST_MOD)
    find_program(DOT dot)
    if(NOT DOT)
        message(FATAL_ERROR "Could not find the dot executable (from Graphviz) required for PostMØD.\n"
                "Disable with -DBUILD_POST_MOD=no, or add the folder of the executable to PATH.")# or give with -DCMAKE_PROGRAM_PATH=paths (currently '${CMAKE_PROGRAM_PATH}').")
    endif()
    message(STATUS "Found dot: ${DOT}")

    execute_process(
            COMMAND ${DOT} -P
            COMMAND grep "cairo_device_svg -> output_svg"
            RESULT_VARIABLE res
            OUTPUT_QUIET ERROR_QUIET)
    if(${res} EQUAL 0)
        message(STATUS "Found dot to support cairo svg output.")
    else()
        message(FATAL_ERROR "The dot command does not seem to support cairo svg output, which is needed for PostMØD."
                " Disable with -DBUILD_POST_MOD=no\n")
    endif()
    execute_process(
            COMMAND ${DOT} -P
            COMMAND grep "cairo_device_pdf -> output_pdf"
            RESULT_VARIABLE res
            OUTPUT_QUIET ERROR_QUIET)
    if(${res} EQUAL 0)
        message(STATUS "Found dot to support cairo pdf output.")
    else()
        message(FATAL_ERROR "The dot command does not seem to support cairo pdf output, which is needed for PostMØD."
                " Disable with -DBUILD_POST_MOD=no\n")
    endif()
    execute_process(
            COMMAND ${DOT} -P
            COMMAND grep "rsvg_loadimage_svg -> render_cairo"
            RESULT_VARIABLE res
            OUTPUT_QUIET ERROR_QUIET)
    if(${res} EQUAL 0)
        message(STATUS "Found dot to support svg to cairo conversion.")
    else()
        message(FATAL_ERROR "The dot command does not seem to support svg to cairo conversion, which is needed for PostMØD."
                " Disable with -DBUILD_POST_MOD=no\n")
    endif()
endif()


# Open Babel
# -------------------------------------------------------------------------
if(WITH_OPENBABEL)
    find_package(OpenBabel3 3)
    if(OpenBabel3_FOUND)
        string(APPEND libmod_config_dependencies "find_dependency(OpenBabel3 3)\n")
        list(APPEND libmod_config_find_files "FindOpenBabel3.cmake")
    else()
        find_package(OpenBabel2 2.3.2)
        if(OpenBabel2_FOUND)
            string(APPEND libmod_config_dependencies "find_dependency(OpenBabel2 2.3.2)\n")
            list(APPEND libmod_config_find_files "FindOpenBabel2.cmake")
        else()
            message(FATAL_ERROR "Could not find Open Babel, neither version 2 nor 3.\n"
                    "Either disable Open Babel with WITH_OPENBABEL=off (not recommended), or add appropriate paths to CMAKE_PREFIX_PATH.")
        endif()
    endif()
    set(MOD_HAVE_OPENBABEL TRUE)
endif()


if(BUILD_POST_MOD)
    # pdf2svg
    # ---------------------------------------------------------------------
    find_program(PDF2SVG pdf2svg)
    if(NOT PDF2SVG)
        message(FATAL_ERROR "Could not find pdf2svg program, required for post_mod.")
    else()
        message(STATUS "Found pdf2svg: ${PDF2SVG}")
    endif()

    # pdflatex
    # ---------------------------------------------------------------------
    find_package(LATEX REQUIRED COMPONENTS PDFLATEX)
endif()


# Python
# -------------------------------------------------------------------------
if(BUILD_PY_MOD)
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
    string(APPEND libmod_config_dependencies "find_dependency(Python3 REQUIRED COMPONENTS Interpreter Development)\n")
endif()


# Sphinx
# -------------------------------------------------------------------------
if(BUILD_DOC)
    find_program(SPHINX NAMES sphinx-build sphinx-build3 sphinx-build-3 sphinx-build2)
    if(SPHINX)
        message(STATUS "Sphinx: ${SPHINX}")
    else()
        message(FATAL_ERROR "Sphinx not found, required for building the documentation. Use --DBUILD_DOC=off to disable.")
    endif()
endif()


# Threads
# -------------------------------------------------------------------------
find_package(Threads REQUIRED)
string(APPEND libmod_config_dependencies "find_dependency(Threads)\n")


# Various Python modules
# -------------------------------------------------------------------------
if(BUILD_EPIM)

endif()



###########################################################################
# Targets and Artefacts
###########################################################################
include(CMakeFiles.txt)
include(Coverage)
include(GNUInstallDirs)

if(NOT hasParent)
    set(CMAKE_NO_BUILTIN_CHRPATH TRUE)                 #
    # https://gitlab.kitware.com/cmake/community/wikis/doc/cmake/RPATH-handling
    set(CMAKE_SKIP_BUILD_RPATH FALSE)                  # use RPATH in the build tree
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)          # but not the RPATH used in installation
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)        # in addition to those deduced automatically
    # use
    # set_target_properties(theTarget PROPERTIES INSTALL_RPATH "theRPATH")
    # on each target that needs it
endif()

add_subdirectory(libs/epim)
add_subdirectory(libs/gml)
add_subdirectory(libs/jla_boost)
add_subdirectory(libs/libmod)
add_subdirectory(libs/pymodutils)
add_subdirectory(libs/pymod)
add_subdirectory(libs/post_mod)
add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(test)


# Packaging
# -------------------------------------------------------------------------
string(TOUPPER ${PROJECT_NAME} PNAME_UPPER)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})  # because the default doesn't include the tweak number
set(CPACK_GENERATOR "DEB")
#set(CPACK_GENERATOR "DEB" "RPM")
set(CPACK_DEB_COMPONENT_INSTALL ON)
#set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# we use CACHE INTERNAL to make it work when in a subproject
set(CPACK_DEBIAN_${PNAME_UPPER}_RUN_PACKAGE_SHLIBDEPS ON                                          CACHE INTERNAL "")
set(CPACK_DEBIAN_${PNAME_UPPER}_LIB_PACKAGE_SHLIBDEPS ON                                          CACHE INTERNAL "")
set(CPACK_DEBIAN_${PNAME_UPPER}_DEV_PACKAGE_SHLIBDEPS ON                                          CACHE INTERNAL "")
set(CPACK_DEBIAN_${PNAME_UPPER}_DOC_PACKAGE_SHLIBDEPS ON                                          CACHE INTERNAL "")

set(CPACK_COMPONENT_${PNAME_UPPER}_RUN_DEPENDS ${PNAME_UPPER}_LIB                                 CACHE INTERNAL "")
set(CPACK_COMPONENT_${PNAME_UPPER}_DEV_DEPENDS ${PNAME_UPPER}_LIB                                 CACHE INTERNAL "")

set(CPACK_DEBIAN_${PNAME_UPPER}_RUN_PACKAGE_NAME    "${PNAME_FILE}"                               CACHE INTERNAL "")
set(CPACK_DEBIAN_${PNAME_UPPER}_LIB_PACKAGE_NAME "lib${PNAME_FILE}"                               CACHE INTERNAL "")
set(CPACK_DEBIAN_${PNAME_UPPER}_DEV_PACKAGE_NAME "lib${PNAME_FILE}-dev"                           CACHE INTERNAL "")
set(CPACK_DEBIAN_${PNAME_UPPER}_DOC_PACKAGE_NAME    "${PNAME_FILE}-doc"                           CACHE INTERNAL "")
set(CPACK_DEBIAN_${PNAME_UPPER}_RUN_FILE_NAME       "${PNAME_FILE}_${PROJECT_VERSION}.deb"        CACHE INTERNAL "")
set(CPACK_DEBIAN_${PNAME_UPPER}_LIB_FILE_NAME    "lib${PNAME_FILE}_${PROJECT_VERSION}.deb"        CACHE INTERNAL "")
set(CPACK_DEBIAN_${PNAME_UPPER}_DEV_FILE_NAME    "lib${PNAME_FILE}-dev_${PROJECT_VERSION}.deb"    CACHE INTERNAL "")
set(CPACK_DEBIAN_${PNAME_UPPER}_DOC_FILE_NAME       "${PNAME_FILE}-doc_${PROJECT_VERSION}.deb"    CACHE INTERNAL "")

#set(CPACK_RPM_${PNAME_UPPER}_RUN_PACKAGE_NAME       "${PNAME_FILE}"                              CACHE INTERNAL "")
#set(CPACK_RPM_${PNAME_UPPER}_LIB_PACKAGE_NAME    "lib${PNAME_FILE}"                              CACHE INTERNAL "")
#set(CPACK_RPM_${PNAME_UPPER}_DEV_PACKAGE_NAME    "lib${PNAME_FILE}-devel"                        CACHE INTERNAL "")
#set(CPACK_RPM_${PNAME_UPPER}_DOC_PACKAGE_NAME       "${PNAME_FILE}-doc"                          CACHE INTERNAL "")
#set(CPACK_RPM_${PNAME_UPPER}_RUN_FILE_NAME          "${PNAME_FILE}-${PROJECT_VERSION}.rpm"       CACHE INTERNAL "")
#set(CPACK_RPM_${PNAME_UPPER}_LIB_FILE_NAME       "lib${PNAME_FILE}-${PROJECT_VERSION}.rpm"       CACHE INTERNAL "")
#set(CPACK_RPM_${PNAME_UPPER}_DEV_FILE_NAME       "lib${PNAME_FILE}-devel-${PROJECT_VERSION}.rpm" CACHE INTERNAL "")
#set(CPACK_RPM_${PNAME_UPPER}_DOC_FILE_NAME          "${PNAME_FILE}-doc-${PROJECT_VERSION}.rpm"   CACHE INTERNAL "")

if(NOT hasParent)
    set(CPACK_SOURCE_PACKAGE_FILE_NAME
            "${PNAME_FILE}-${CPACK_PACKAGE_VERSION}")
    set(CPACK_SOURCE_GENERATOR "TGZ")
    set(CPACK_SOURCE_IGNORE_FILES
            "\.swp$"
            "~$"
            "/__pycache__/"
            "\.pyc$"
            "/\.git(/|$)"
            "/\.gitmodules$"
            "/\.idea/"
            "/\.mypy_cache/"
            "/build/"
            "/stage/"
            "/conda/conda-bld/"
            "/test/py/out/"
            "/test/py/summary/"
            "/test/py/.gdb_history$"
            "/test/py/.*/out/"
            "/test/py/.*/summary/"
            "/test/py/.*/.gdb_history$"
            "/test/py/graph/myGraph.gml$"
            "/test/py/rule/myRule.gml$"
            "/bootstrap.sh$")
    add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
    include(CPack)
endif()
