add_custom_target(indigo-dotnet
        COMMAND dotnet test -c Release
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/bin/Release/*.nupkg ${DIST_DIRECTORY}/
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
add_dependencies(indigo-wrappers indigo-dotnet)
if (BUILD_INDIGO)
    add_dependencies(indigo-dotnet before-indigo-wrappers)
endif ()
