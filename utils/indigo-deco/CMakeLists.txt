cmake_minimum_required(VERSION 3.4)

project(indigo-deco LANGUAGES C)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
target_link_libraries(${PROJECT_NAME} indigo)

add_test(NAME deco-simple-test COMMAND ${PROJECT_NAME} -h)
