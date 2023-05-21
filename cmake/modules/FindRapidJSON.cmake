find_path(
        RapidJSON_INCLUDE_DIRS
        NAMES rapidjson/rapidjson.h
        PATHS ${RapidJSON_INCLUDEDIR}
        DOC "Include directory for the rapidjson library."
)

if(RapidJSON_INCLUDE_DIRS)
    if (EXISTS "${RapidJSON_INCLUDE_DIRS}/rapidjson/rapidjson.h")
        file(READ "${RapidJSON_INCLUDE_DIRS}/rapidjson/rapidjson.h" RapidJSON_VERSION_CONTENT)
        string(REGEX MATCH "#define +RAPIDJSON_MAJOR_VERSION +([0-9]+)" _dummy "${RapidJSON_VERSION_CONTENT}")
        set(RapidJSON_VERSION_MAJOR "${CMAKE_MATCH_1}")

        string(REGEX MATCH "#define +RAPIDJSON_MINOR_VERSION +([0-9]+)" _dummy "${RapidJSON_VERSION_CONTENT}")
        set(RapidJSON_VERSION_MINOR "${CMAKE_MATCH_1}")

        string(REGEX MATCH "#define +RAPIDJSON_PATCH_VERSION +([0-9]+)" _dummy "${RapidJSON_VERSION_CONTENT}")
        set(RapidJSON_VERSION_PATCH "${CMAKE_MATCH_1}")

        set(RapidJSON_VERSION "${RapidJSON_VERSION_MAJOR}.${RapidJSON_VERSION_MINOR}.${RapidJSON_VERSION_PATCH}")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RapidJSON REQUIRED_VARS RapidJSON_INCLUDE_DIRS
                                            VERSION_VAR RapidJSON_VERSION)

if(RapidJSON_FOUND)
    if(NOT TARGET RapidJSON::RapidJSON)
        add_library(RapidJSON::RapidJSON INTERFACE)
        set_target_properties(RapidJSON PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${RapidJSON_INCLUDE_DIRS}")
    endif()
endif()
