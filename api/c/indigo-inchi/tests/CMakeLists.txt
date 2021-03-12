cmake_minimum_required(VERSION 3.6)

project(indigo-inchi-tests-c LANGUAGES C)

if (ENABLE_TESTS)
    add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/indigo-inchi-test.c)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC indigo
            PUBLIC indigo-inchi)
    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
endif()
