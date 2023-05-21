include(FindPackageHandleStandardArgs)

find_library(TinyXML2_LIBRARY NAMES tinyxml2)
find_path(TinyXML2_INCLUDE_DIR NAMES tinyxml2.h)

if (TinyXML2_INCLUDE_DIR)
    if (EXISTS "${TinyXML2_INCLUDE_DIR}/tinyxml2.h")
        FILE(READ "${TinyXML2_INCLUDE_DIR}/tinyxml2.h" TinyXML2_VERSION_CONTENT)

        STRING(REGEX MATCH "const int TIXML2_MAJOR_VERSION = ([0-9]+)" _dummy "${TinyXML2_VERSION_CONTENT}")
        SET(TinyXML2_VERSION_MAJOR "${CMAKE_MATCH_1}")

        STRING(REGEX MATCH "const int TIXML2_MINOR_VERSION = ([0-9]+)" _dummy "${TinyXML2_VERSION_CONTENT}")
        SET(TinyXML2_VERSION_MINOR "${CMAKE_MATCH_1}")

        STRING(REGEX MATCH "const int TIXML2_PATCH_VERSION = ([0-9]+)" _dummy "${TinyXML2_VERSION_CONTENT}")
        SET(TinyXML2_VERSION_PATCH "${CMAKE_MATCH_1}")

        SET(TinyXML2_VERSION "${TinyXML2_VERSION_MAJOR}.${TinyXML2_VERSION_MINOR}.${TinyXML2_VERSION_PATCH}")
    endif ()
endif ()

find_package_handle_standard_args(TinyXML2 REQUIRED_VARS TinyXML2_LIBRARY TinyXML2_INCLUDE_DIR VERSION_VAR TinyXML2_VERSION)

if (TinyXML2_FOUND)
    mark_as_advanced(TinyXML2_INCLUDE_DIR)
    mark_as_advanced(TinyXML2_LIBRARY)
endif()

if (TinyXML2_FOUND AND NOT TARGET TinyXML2::TinyXML2)
    add_library(TinyXML2::TinyXML2 IMPORTED UNKNOWN)
    set_target_properties(TinyXML2::TinyXML2 PROPERTIES IMPORTED_LOCATION ${TinyXML2_LIBRARY})
    target_include_directories(TinyXML2::TinyXML2 INTERFACE ${TinyXML2_INCLUDE_DIR})
endif()
