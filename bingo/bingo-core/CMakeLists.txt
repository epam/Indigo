cmake_minimum_required(VERSION 2.8)

project(BingoCore)

SET(BingoCore_HEADERS_DIR ${BingoCore_SOURCE_DIR}/src CACHE INTERNAL "include directories" FORCE)
CMAKE_POLICY(SET CMP0011 NEW)
INCLUDE(${Bingo_SOURCE_DIR}/bingo-version.cmake)                                            
MESSAGE(STATUS "BingoCore version is ${BINGO_VERSION}")
configure_file(${BingoCore_HEADERS_DIR}/core/bingo_version.h.in ${BingoCore_HEADERS_DIR}/core/bingo_version.h)

file(GLOB BingoCore_src src/core/*.cpp)
file(GLOB BingoCore_headers ${BingoCore_HEADERS_DIR}/core/*.h)

include(SetBuildParameters)
include_directories(src ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)
add_library(bingo-core STATIC ${BingoCore_src} ${BingoCore_headers})
set_target_properties(bingo-core PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

set_property(TARGET bingo-core PROPERTY FOLDER "bingo-core")

