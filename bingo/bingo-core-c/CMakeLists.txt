cmake_minimum_required(VERSION 2.6)

project(BingoCoreC C)

file(GLOB BingoCoreC_src ../src/core-c/*.cpp)
file(GLOB BingoCoreC_headers ../src/core-c/*.h)

include_directories(../src ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/.. ${ZLib_HEADERS_DIR})

add_library(bingo-core-c STATIC ${BingoCoreC_src} ${BingoCoreC_headers})
set_target_properties(bingo-core-c PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

SET_TARGET_PROPERTIES(bingo-core-c PROPERTIES LINKER_LANGUAGE C)
set_property(TARGET bingo-core-c PROPERTY FOLDER "bingo-core-c")



