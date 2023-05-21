include(FindPackageHandleStandardArgs)

find_library(Cairo_LIBRARY NAMES cairo)
find_path(Cairo_INCLUDE_DIR NAMES cairo.h)


IF (Cairo_INCLUDE_DIRS)
    IF (EXISTS "${Cairo_INCLUDE_DIRS}/cairo-version.h")
        FILE(READ "${Cairo_INCLUDE_DIRS}/cairo-version.h" Cairo_VERSION_CONTENT)

        STRING(REGEX MATCH "#define +CAIRO_VERSION_MAJOR +([0-9]+)" _dummy "${Cairo_VERSION_CONTENT}")
        SET(Cairo_VERSION_MAJOR "${CMAKE_MATCH_1}")

        STRING(REGEX MATCH "#define +CAIRO_VERSION_MINOR +([0-9]+)" _dummy "${Cairo_VERSION_CONTENT}")
        SET(Cairo_VERSION_MINOR "${CMAKE_MATCH_1}")

        STRING(REGEX MATCH "#define +CAIRO_VERSION_MICRO +([0-9]+)" _dummy "${Cairo_VERSION_CONTENT}")
        SET(Cairo_VERSION_MICRO "${CMAKE_MATCH_1}")

        SET(Cairo_VERSION "${Cairo_VERSION_MAJOR}.${Cairo_VERSION_MINOR}.${Cairo_VERSION_MICRO}")
    ENDIF ()
ENDIF ()

find_package_handle_standard_args(Cairo REQUIRED_VARS Cairo_LIBRARY Cairo_INCLUDE_DIR VERSION_VAR Cairo_VERSION)

if (Cairo_FOUND)
    mark_as_advanced(Cairo_INCLUDE_DIR)
    mark_as_advanced(Cairo_LIBRARY)
endif()

if (Cairo_FOUND AND NOT TARGET Cairo::Cairo)
    add_library(Cairo::Cairo IMPORTED UNKNOWN)
    set_target_properties(Cairo::Cairo PROPERTIES IMPORTED_LOCATION ${Cairo_LIBRARY})
    target_include_directories(Cairo::Cairo INTERFACE ${Cairo_INCLUDE_DIR})
endif()
