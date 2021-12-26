cmake_minimum_required(VERSION 3.4)

project(z C)

file (GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS "src/*.c")

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME} SYSTEM
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if (NOT MSVC)
    target_compile_definitions(${PROJECT_NAME}
            PRIVATE -DHAVE_UNISTD_H)
endif()

message(STATUS "Using local version of ${PROJECT_NAME}")
