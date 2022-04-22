cmake_minimum_required(VERSION 3.4)

project(png C)

file (GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS "src/*.c")

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME} SYSTEM
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} z)

if (CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL aarch64)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DPNG_ARM_NEON_OPT=0)
endif()

message(STATUS "Using local version of ${PROJECT_NAME}")
