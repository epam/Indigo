find_program(MAVEN mvnd mvnw mvn
        PATHS ENV PATH)
if (NOT MAVEN)
    if (NOT DEFINED ENV{JAVA_HOME})
        message(WARNING "JAVA_HOME executable not found, using of built-in mvnw is not possible")
    else ()
        find_program(MAVEN mvnw PATHS ${CMAKE_CURRENT_SOURCE_DIR})
    endif ()
endif ()

if (NOT MAVEN)
    message(WARNING "Maven executable (mvn or mvnd or mvnw) not found, bingo-elastic-java could not be built")
else ()
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} NATIVE_CURRENT_SOURCE_DIR)

    message(STATUS "Found Maven: ${MAVEN}")
    add_custom_target(bingo-elastic-java
            COMMAND ${MAVEN} install
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_custom_command(TARGET bingo-elastic-java POST_BUILD
            COMMAND ${COPY_COMMAND} ${NATIVE_CURRENT_SOURCE_DIR}${SEP}target${SEP}*.jar ${NATIVE_DIST_DIRECTORY}${SEP}
            )
    if (BUILD_INDIGO)
        add_dependencies(bingo-elastic-java indigo-java)
    endif ()
endif ()
