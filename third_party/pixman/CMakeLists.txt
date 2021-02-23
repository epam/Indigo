cmake_minimum_required(VERSION 3.4)
project(pixman C)

file (GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS "pixman/*.c" "pixman/dither/*.c")
list(REMOVE_ITEM ${PROJECT_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/pixman/pixman-region.c)

if (APPLE)
    add_definitions(-DHAVE_PTHREADS)
    list(REMOVE_ITEM ${PROJECT_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/pixman/pixman-timer.c)
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
target_compile_definitions(${PROJECT_NAME}
        PRIVATE -DPACKAGE)
if (NOT EMSCRIPTEN)
    target_compile_definitions(${PROJECT_NAME}
            PRIVATE -DHAVE_PTHREADS)
else()
    target_compile_definitions(${PROJECT_NAME}
            PRIVATE -DPIXMAN_NO_TLS)
endif()

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pixman)

message(STATUS "Using local version of ${PROJECT_NAME}")
