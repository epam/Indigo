cmake_minimum_required(VERSION 3.4)

project(tinyxml CXX)

file (GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME} SYSTEM
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

message(STATUS "Using local version of ${PROJECT_NAME}")
