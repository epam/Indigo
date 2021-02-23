cmake_minimum_required(VERSION 3.4)

project(bingo-core-c LANGUAGES CXX)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME}-object OBJECT ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME}-object
        PUBLIC bingo-core)

add_library(${PROJECT_NAME}-static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
target_link_libraries(${PROJECT_NAME}-static
        PUBLIC bingo-core)
target_include_directories(${PROJECT_NAME}-static
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
target_link_libraries(${PROJECT_NAME}
        PUBLIC bingo-core)
