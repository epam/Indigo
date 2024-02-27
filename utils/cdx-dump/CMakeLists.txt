cmake_minimum_required(VERSION 3.4)

project(cdx-dump LANGUAGES CXX)

if (MSVC)
    string(APPEND CMAKE_CXX_FLAGS " -W4 -WX")
endif ()

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(${PROJECT_NAME}
        PUBLIC indigo-core)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}/utils
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${DIST_DIRECTORY}/utils/
)
