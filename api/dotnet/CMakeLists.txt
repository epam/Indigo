add_custom_target(bindings-dotnet
        COMMAND dotnet test -c Release
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/bin/Release/*.nupkg ${DIST_DIRECTORY}/
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
add_dependencies(bindings bindings-dotnet)
