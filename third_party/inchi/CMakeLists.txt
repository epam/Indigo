cmake_minimum_required(VERSION 3.6)

project(inchi LANGUAGES C)

file (GLOB ${PROJECT_NAME}_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/INCHI_BASE/src/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/INCHI_API/libinchi/src/*.c)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
target_compile_definitions(${PROJECT_NAME} PUBLIC -DTARGET_API_LIB)
target_include_directories(${PROJECT_NAME} SYSTEM
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/INCHI_BASE/src
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/INCHI_API/libinchi/src)

if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME}
            PRIVATE -Wno-pointer-bool-conversion
            PRIVATE -Wno-parentheses
            PRIVATE -Wno-sizeof-pointer-memaccess
            PRIVATE -Wno-tautological-compare
            PRIVATE -Wno-unsequenced
            PRIVATE -Wno-comment
            PRIVATE -Wno-logical-op-parentheses)
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME}
            PRIVATE -Wno-format-overflow)
endif()

message(STATUS "Using local version of ${PROJECT_NAME}")
