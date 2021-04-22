find_program(DOTNET dotnet PATHS ENV PATH)

if (NOT DOTNET)
    message(WARNING ".NET executable (dotnet) not found, indigo-dotnet could not be built")
else()
    message(STATUS "Found .NET: ${DOTNET}")
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} NATIVE_CURRENT_SOURCE_DIR)
    add_custom_target(indigo-dotnet
            COMMAND ${DOTNET} test -c Release
            COMMAND ${COPY_COMMAND} ${NATIVE_CURRENT_SOURCE_DIR}${SEP}src${SEP}bin${SEP}Release${SEP}*.nupkg ${NATIVE_DIST_DIRECTORY}${SEP}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(indigo-wrappers indigo-dotnet)
    if (BUILD_INDIGO)
        add_dependencies(indigo-dotnet before-indigo-wrappers)
    endif ()
endif()
