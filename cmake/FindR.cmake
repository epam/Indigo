find_path(
    R_INCLUDE_DIRS
    NAMES R.h
    PATHS ${R_INCLUDE_DIR} /usr/share/R/include
    DOC "Include directory for the R language"
    REQUIRED
)

find_library(R_LIBRARIES
        NAMES R
        ${PC_CAIRO_LIBRARY_DIRS})

find_program (R_EXECUTABLE
        NAMES R
        REQUIRED)

if(R_INCLUDE_DIRS)
    if (EXISTS "${R_INCLUDE_DIRS}/Rversion.h")
        file(READ "${R_INCLUDE_DIRS}/Rversion.h" R_VERSION_CONTENT)
        string(REGEX MATCH "#define R_MAJOR  \"([0-9]+)\"" _dummy "${R_VERSION_CONTENT}")
        set(R_VERSION_MAJOR "${CMAKE_MATCH_1}")

        string(REGEX MATCH "#define R_MINOR  \"([0-9]+)\\.([0-9]+)\"" _dummy "${R_VERSION_CONTENT}")
        set(R_VERSION_MINOR "${CMAKE_MATCH_1}")
        set(R_VERSION_PATCH "${CMAKE_MATCH_2}")

        set(R_VERSION "${R_VERSION_MAJOR}.${R_VERSION_MINOR}.${R_VERSION_PATCH}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(R REQUIRED_VARS R_LIBRARIES R_EXECUTABLE R_INCLUDE_DIRS
                                    VERSION_VAR R_VERSION)

if(R_FOUND)
    if(NOT TARGET R::R)
        add_library(R::R UNKNOWN IMPORTED)
        set_target_properties(R::R PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${R_INCLUDE_DIRS}")

        if(R_LIBRARY_RELEASE)
            set_property(TARGET R::R APPEND PROPERTY
                    IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(R::R PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
                    IMPORTED_LOCATION_RELEASE "${R_LIBRARY_RELEASE}")
        endif()

        if(R_LIBRARY_DEBUG)
            set_property(TARGET R::R APPEND PROPERTY
                    IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(R::R PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
                    IMPORTED_LOCATION_DEBUG "${R_LIBRARY_DEBUG}")
        endif()

        if(NOT R_LIBRARY_RELEASE AND NOT R_LIBRARY_DEBUG)
            set_target_properties(R::R PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                    IMPORTED_LOCATION "${R_LIBRARIES}")
        endif()
    endif()
endif()
