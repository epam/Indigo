include(FindPackageHandleStandardArgs)

find_library(InChI_LIBRARY NAMES inchi)
find_path(InChI_INCLUDE_DIR NAMES inchi_api.h)

if (InChI_INCLUDE_DIR)
    if (EXISTS "${InChI_INCLUDE_DIR}/inchi_api.h")
        file(READ "${InChI_INCLUDE_DIR}/inchi_api.h" InChI_VERSION_CONTENT)
        string(REGEX MATCH "Software version +([0-9]+\\.[0-9]+)" _dummy "${InChI_VERSION_CONTENT}")
        set(InChI_VERSION "${CMAKE_MATCH_1}")
    endif ()
endif ()

find_package_handle_standard_args(InChI REQUIRED_VARS InChI_LIBRARY InChI_INCLUDE_DIR VERSION_VAR InChI_VERSION)

if (InChI_FOUND)
    mark_as_advanced(InChI_INCLUDE_DIR)
    mark_as_advanced(InChI_LIBRARY)
endif()

if (InChI_FOUND AND NOT TARGET InChI::InChI)
    add_library(InChI::InChI IMPORTED UNKNOWN)
    set_target_properties(InChI::InChI PROPERTIES IMPORTED_LOCATION ${InChI_LIBRARY})
    target_include_directories(InChI::InChI INTERFACE ${InChI_INCLUDE_DIR})
endif()
