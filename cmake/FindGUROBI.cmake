find_path(GUROBI_INCLUDE_DIRS
    NAMES gurobi_c.h
    HINTS ${GUROBI_DIR} $ENV{GUROBI_HOME}
    PATH_SUFFIXES include)

find_library(GUROBI_LIBRARY
    NAMES gurobi gurobi100 gurobi110 gurobi120 gurobi130 gurobi140 gurobi150 gurobi160 gurobi170 gurobi180 gurobi190
    HINTS ${GUROBI_DIR} $ENV{GUROBI_HOME}
    PATH_SUFFIXES lib)

if(MSVC)
	set(MSVC_YEAR "2017")
	
	if(MT)
		set(M_FLAG "mt")
	else()
		set(M_FLAG "md")
	endif()
	
	find_library(GUROBI_CXX_LIBRARY
		NAMES gurobi_c++${M_FLAG}${MSVC_YEAR}
		HINTS ${GUROBI_DIR} $ENV{GUROBI_HOME}
		PATH_SUFFIXES lib)
	find_library(GUROBI_CXX_DEBUG_LIBRARY
		NAMES gurobi_c++${M_FLAG}d${MSVC_YEAR}
		HINTS ${GUROBI_DIR} $ENV{GUROBI_HOME}
		PATH_SUFFIXES lib)
else()
	find_library(GUROBI_CXX_LIBRARY
		NAMES gurobi_c++
		HINTS ${GUROBI_DIR} $ENV{GUROBI_HOME}
		PATH_SUFFIXES lib)
	set(GUROBI_CXX_DEBUG_LIBRARY ${GUROBI_CXX_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GUROBI DEFAULT_MSG GUROBI_INCLUDE_DIRS GUROBI_LIBRARY GUROBI_CXX_LIBRARY)


add_library(Gurobi SHARED IMPORTED GLOBAL)
set_target_properties(Gurobi PROPERTIES
	IMPORTED_LOCATION "${GUROBI_LIBRARY}"
	INTERFACE_INCLUDE_DIRECTORIES "${GUROBI_INCLUDE_DIRS}"
)

add_library(Gurobi::CXX STATIC IMPORTED GLOBAL)
target_link_libraries(Gurobi::CXX INTERFACE Gurobi)
set_target_properties(Gurobi::CXX PROPERTIES
	IMPORTED_LOCATION "${GUROBI_CXX_LIBRARY}"
	INTERFACE_INCLUDE_DIRECTORIES "${GUROBI_INCLUDE_DIRS}"
)
