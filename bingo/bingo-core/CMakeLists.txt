cmake_minimum_required(VERSION 3.4)

project(bingo-core LANGUAGES CXX)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/core/bingo_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/core/bingo_version.h)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp)

add_library(${PROJECT_NAME} OBJECT ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME}
        PUBLIC indigo-core)
target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src/core)
