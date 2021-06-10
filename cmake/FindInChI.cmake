# - Try to find InChI
# Once done, this will define
#
#  INCHI_FOUND - system has InChI
#  INCHI_INCLUDE_DIRS - the InChI include directories
#  INCHI_LIBRARIES - link these to use InChI

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_INCHI inchi QUIET)

FIND_PATH(INCHI_INCLUDE_DIRS
        NAMES inchi_api.h
        HINTS ${PC_INCHI_INCLUDEDIR}
        ${PC_INCHI_INCLUDE_DIRS}
        PATH_SUFFIXES inchi
        )

FIND_LIBRARY(INCHI_LIBRARIES
        NAMES inchi
        HINTS ${PC_INCHI_LIBDIR}
        ${PC_INCHI_LIBRARY_DIRS}
        )

if (INCHI_INCLUDE_DIRS)
    if (EXISTS "${INCHI_INCLUDE_DIRS}/inchi_api.h")
        file(READ "${INCHI_INCLUDE_DIRS}/inchi_api.h" INCHI_VERSION_CONTENT)
        string(REGEX MATCH "Software version +([0-9]+\\.[0-9]+)" _dummy "${INCHI_VERSION_CONTENT}")
        set(INCHI_VERSION "${CMAKE_MATCH_1}")
    endif ()
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(InChI REQUIRED_VARS INCHI_LIBRARIES INCHI_INCLUDE_DIRS
                                        VERSION_VAR INCHI_VERSION)

if(InChI_FOUND)
    if(NOT TARGET InChI::Cairdisuo)
        add_library(InChI::InChI UNKNOWN IMPORTED)
        set_target_properties(InChI::InChI PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${InChI_INCLUDE_DIRS}")

        if(InChI_LIBRARY_RELEASE)
            set_property(TARGET InChI::InChI APPEND PROPERTY
                    IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(InChI::InChI PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
                    IMPORTED_LOCATION_RELEASE "${InChI_LIBRARY_RELEASE}")
        endif()

        if(InChI_LIBRARY_DEBUG)
            set_property(TARGET InChI::InChI APPEND PROPERTY
                    IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(InChI::InChI PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
                    IMPORTED_LOCATION_DEBUG "${InChI_LIBRARY_DEBUG}")
        endif()

        if(NOT InChI_LIBRARY_RELEASE AND NOT InChI_LIBRARY_DEBUG)
            set_target_properties(InChI::InChI PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                    IMPORTED_LOCATION "${INCHI_LIBRARIES}")
        endif()
    endif()
endif()
