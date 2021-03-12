cmake_minimum_required(VERSION 3.6)

project(bingo-nosql-tests-c LANGUAGES C)

if (ENABLE_TESTS)
    add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/bingo-nosql-test.c)
    target_link_libraries(${PROJECT_NAME} indigo bingo-nosql)
    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
endif()
