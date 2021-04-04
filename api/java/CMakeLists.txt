find_program(MAVEN_COMMAND mvnd mvnw mvn REQUIRED)

add_custom_target(indigo-java 
    COMMAND ${MAVEN_COMMAND} install 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_command(TARGET indigo-java POST_BUILD
       COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/indigo/target/*.jar ${DIST_DIRECTORY}
       COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/indigo-inchi/target/*.jar ${DIST_DIRECTORY}
       COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/indigo-renderer/target/*.jar ${DIST_DIRECTORY}
       COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/bingo-nosql/target/*.jar ${DIST_DIRECTORY}
)
add_dependencies(indigo-wrappers indigo-java)
if (BUILD_INDIGO)
    add_dependencies(indigo-java before-indigo-wrappers)
endif ()
