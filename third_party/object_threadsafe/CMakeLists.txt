cmake_minimum_required(VERSION 3.4)

project(object_threadsafe)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} SYSTEM
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "Using local version of ${PROJECT_NAME}")
