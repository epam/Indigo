cmake_minimum_required(VERSION 3.4)

project(indigo-depict LANGUAGES C)

add_executable(indigo-depict ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
target_link_libraries(indigo-depict indigo-static indigo-renderer-static)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}/utils
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${DIST_DIRECTORY}/utils/
)

add_test(NAME depict-simple-test COMMAND ${PROJECT_NAME} - "[CH2:1]=[CH:2][CH2:1][CH2:3][C:4](C)[CH2:3]" output.png)
add_test(NAME depict-query-test COMMAND ${PROJECT_NAME} - "CC.[O-][*-]([O-])=O" query.png -query)
