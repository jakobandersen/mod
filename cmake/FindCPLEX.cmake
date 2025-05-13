# Based on https://github.com/ampl/mp/blob/master/support/cmake/FindCPLEX.cmake
#
# Try to find the CPLEX, Concert, IloCplex and CP Optimizer libraries.
# Use CPLEX_DIR to specify the location.
#
# Once done this will add the following imported targets:
#
#   CPLEX::Concert
#   CPLEX::IloCplex
#   CPLEX::Cplex

set(FPHSA_NAME_MISMATCHED 1)
include(FindPackageHandleStandardArgs)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPLEX_ARCH x86-64)
else()
    set(CPLEX_ARCH x86)
endif()

set(CPLEX_LIB_PATHS
        "lib/${CPLEX_ARCH}_sles10_4.1/static_pic"
        "lib/x86-64_osx/static_pic"
        "lib/arm64_osx/static_pic"
        # CPLEX 12.6 has different paths
        "lib/${CPLEX_ARCH}_linux/static_pic"
)

# CPLEX
# -----------------------------------------------------------------------------

find_path(CPLEX_INCLUDE_DIR ilcplex/cplex.h PATHS ${CPLEX_DIR}/cplex/include)
find_library(CPLEX_LIBRARY cplex
        PATHS ${CPLEX_DIR}/cplex PATH_SUFFIXES ${CPLEX_LIB_PATHS})

find_package_handle_standard_args(CPLEX
        "Could not find the CPLEX library. Set CPLEX_DIR to the installation root, currently set to'${CPLEX_DIR}'."
        CPLEX_LIBRARY CPLEX_INCLUDE_DIR)
mark_as_advanced(CPLEX_INCLUDE_DIR CPLEX_LIBRARY)

if(CPLEX_FOUND AND NOT TARGET CPLEX::Cplex)
    #set(CPLEX_LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
    #check_library_exists(m floor "" HAVE_LIBM)
    #if (HAVE_LIBM)
    #    set(CPLEX_LINK_LIBRARIES ${CPLEX_LINK_LIBRARIES} m)
    #endif ()
    add_library(CPLEX::Cplex STATIC IMPORTED GLOBAL)
    set_target_properties(CPLEX::Cplex PROPERTIES
            IMPORTED_LOCATION "${CPLEX_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${CPLEX_INCLUDE_DIR}")
    if(NOT "x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xAppleClang")
        set_target_properties(CPLEX::Cplex PROPERTIES
                INTERFACE_LINK_LIBRARIES ${CMAKE_DL_LIBS})
    endif()
endif()


# ILOCPLEX
# -----------------------------------------------------------------------------

find_path(ILOCPLEX_INCLUDE_DIR ilcplex/ilocplex.h PATHS ${CPLEX_DIR}/cplex/include)
find_library(ILOCPLEX_LIBRARY ilocplex
        PATHS ${CPLEX_DIR}/cplex PATH_SUFFIXES ${CPLEX_LIB_PATHS})

find_package_handle_standard_args(ILOCPLEX
        "Could not find the ILOCPLEX library, part of CPLEX. Set CPLEX_DIR to the installation root, currently set to'${CPLEX_DIR}'."
        ILOCPLEX_LIBRARY ILOCPLEX_INCLUDE_DIR)
mark_as_advanced(ILOCPLEX_INCLUDE_DIR ILOCPLEX_LIBRARY)

if(ILOCPLEX_FOUND AND NOT TARGET CPLEX::IloCplex)
    add_library(CPLEX::IloCplex STATIC IMPORTED GLOBAL)
    set_target_properties(CPLEX::IloCplex PROPERTIES
            IMPORTED_LOCATION "${ILOCPLEX_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ILOCPLEX_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES CPLEX::Cplex)
endif()


# Concert
# -----------------------------------------------------------------------------

find_path(CONCERT_INCLUDE_DIR ilconcert/ilosys.h PATHS ${CPLEX_DIR}/concert/include)
find_library(CONCERT_LIBRARY concert
        PATHS ${CPLEX_DIR}/concert PATH_SUFFIXES ${CPLEX_LIB_PATHS})

find_package_handle_standard_args(CONCERT
        "Could not find the Concert library, part of CPLEX. Set CPLEX_DIR to the installation root, currently set to'${CPLEX_DIR}'."
        CONCERT_LIBRARY CONCERT_INCLUDE_DIR)
mark_as_advanced(CONCERT_INCLUDE_DIR CONCERT_LIBRARY)

if(CONCERT_FOUND AND NOT TARGET CPLEX::Concert)
    add_library(CPLEX::Concert STATIC IMPORTED GLOBAL)
    set_target_properties(CPLEX::Concert PROPERTIES
            IMPORTED_LOCATION "${CONCERT_LIBRARY}"
            #INTERFACE_COMPILE_DEFINITIONS IL_STD
            INTERFACE_INCLUDE_DIRECTORIES "${CONCERT_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES CPLEX::IloCplex
            )
endif()
