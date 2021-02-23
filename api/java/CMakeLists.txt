add_custom_target(bindings-java
        COMMAND mvn install
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/indigo/target/*.jar ${DIST_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/indigo-inchi/target/*.jar ${DIST_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/indigo-renderer/target/*.jar ${DIST_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/bingo-nosql/target/*.jar ${DIST_DIRECTORY}/
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
add_dependencies(bindings bindings-java)
