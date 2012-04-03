cmake_minimum_required(VERSION 2.6)

project(BingoCoreC C)

file(GLOB BingoCoreC_src ../src/core-c/*.cpp)
file(GLOB BingoCoreC_headers ../src/core-c/*.h)
set(BingoCoreC_HEADERS_DIR ../src/core-c/)                                 
include(../bingo-version.cmake)
MESSAGE(STATUS "BingoCoreC version is ${BINGO_VERSION}")
configure_file(${BingoCoreC_SOURCE_DIR}/../src/core/bingo_version.h.in ${BingoCoreC_SOURCE_DIR}/../src/core/bingo_version.h)

include(SetBuildParameters)

include_directories(../src ../src/core ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/.. ${ZLib_HEADERS_DIR})

add_library(bingo-core-c STATIC ${BingoCoreC_src} ${BingoCoreC_headers})
add_library(bingo-core-c-shared SHARED ${BingoCoreC_src} ${BingoCoreC_headers})
target_link_libraries(bingo-core-c-shared molecule reaction layout bingo-core)

set_target_properties(bingo-core-c PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
SET_TARGET_PROPERTIES(bingo-core-c PROPERTIES LINKER_LANGUAGE C)
set_property(TARGET bingo-core-c PROPERTY FOLDER "bingo-core-c")

set_target_properties(bingo-core-c-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
SET_TARGET_PROPERTIES(bingo-core-c-shared PROPERTIES LINKER_LANGUAGE C)

set_property(TARGET bingo-core-c-shared PROPERTY FOLDER "bingo-core-c")
set_property(TARGET bingo-core-c-shared PROPERTY OUTPUT_NAME "bingo-core-c")



