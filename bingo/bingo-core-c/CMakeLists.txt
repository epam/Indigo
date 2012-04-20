cmake_minimum_required(VERSION 2.6)

project(BingoCoreC)

file(GLOB BingoCoreC_src src/*.cpp)
file(GLOB BingoCoreC_headers src/*.h)

set(BingoCoreC_HEADERS_DIR ${BingoCoreC_SOURCE_DIR}/src CACHE INTERNAL "include directories" FORCE)

include(SetBuildParameters)

include_directories(../src ../src/core ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/.. ${ZLib_HEADERS_DIR} ${BingoCore_HEADERS_DIR})

add_library(bingo-core-c STATIC ${BingoCoreC_src} ${BingoCoreC_headers})
add_library(bingo-core-c-shared SHARED ${BingoCoreC_src} ${BingoCoreC_headers})
target_link_libraries(bingo-core-c bingo-core)
target_link_libraries(bingo-core-c-shared molecule reaction layout bingo-core)

set_target_properties(bingo-core-c PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
set_property(TARGET bingo-core-c PROPERTY FOLDER "bingo-core-c")

set_target_properties(bingo-core-c-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

set_property(TARGET bingo-core-c PROPERTY OUTPUT_NAME "bingo-core-c-static")
set_property(TARGET bingo-core-c-shared PROPERTY FOLDER "bingo-core-c-shared")
set_property(TARGET bingo-core-c-shared PROPERTY OUTPUT_NAME "bingo-core-c")
