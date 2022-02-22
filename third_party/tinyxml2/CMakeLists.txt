cmake_minimum_required(VERSION 3.4)

project(tinyxml2 CXX)

file (GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME} SYSTEM
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

message(STATUS "Using local version of ${PROJECT_NAME}")
