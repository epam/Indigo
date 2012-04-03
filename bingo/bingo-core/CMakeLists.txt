cmake_minimum_required(VERSION 2.6)

project(BingoCore)

INCLUDE(${Bingo_SOURCE_DIR}/bingo-version.cmake)                                            
MESSAGE(STATUS "BingoCore version is ${BINGO_VERSION}")
configure_file(${BingoCore_SOURCE_DIR}/../src/core/bingo_version.h.in ${BingoCore_SOURCE_DIR}/../src/core/bingo_version.h)
file(GLOB BingoCore_src ../src/core/*.cpp)
file(GLOB BingoCore_headers ../src/core/*.h)
set(BingoCore_HEADERS_DIR ../src/core)

include(SetBuildParameters)
include_directories(../src ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)
add_library(bingo-core STATIC ${BingoCore_src} ${BingoCore_headers})
set_target_properties(bingo-core PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

set_property(TARGET bingo-core PROPERTY FOLDER "bingo-core")

