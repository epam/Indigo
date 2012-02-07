cmake_minimum_required(VERSION 2.6)

project(BingoCore)

file(GLOB BingoCore_src ../src/core/*.cpp)
file(GLOB BingoCore_headers ../src/core/*.h)

include_directories(../src ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..})

add_library(bingo-core STATIC ${BingoCore_src} ${BingoCore_headers})

set_property(TARGET bingo-core PROPERTY FOLDER "bingo-core")

