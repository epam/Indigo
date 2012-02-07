cmake_minimum_required(VERSION 2.6)

project(BingoOracleSolution)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

INCLUDE(ConfigureCommon)

add_subdirectory(../../zlib-src "${CMAKE_CURRENT_BINARY_DIR}/zlib")
# Get defined variable with the headers
get_directory_property(ZLib_HEADERS_DIR DIRECTORY ../../zlib-src DEFINITION ZLib_HEADERS_DIR)

add_subdirectory(../../common "${CMAKE_CURRENT_BINARY_DIR}/common")

add_subdirectory(../../tinyxml "${CMAKE_CURRENT_BINARY_DIR}/tinyxml")
# Get defined variable with the headers
get_directory_property(TinyXML_HEADERS_DIR DIRECTORY ../../tinyxml DEFINITION TinyXML_HEADERS_DIR)

add_subdirectory(../../graph "${CMAKE_CURRENT_BINARY_DIR}/graph")
add_subdirectory(../../molecule "${CMAKE_CURRENT_BINARY_DIR}/molecule")
add_subdirectory(../../reaction "${CMAKE_CURRENT_BINARY_DIR}/reaction")

add_subdirectory(../../layout "${CMAKE_CURRENT_BINARY_DIR}/layout")

add_subdirectory(../../bingo/bingo-core-c "${CMAKE_CURRENT_BINARY_DIR}/bingo-core-c")
add_subdirectory(../../bingo/bingo-core "${CMAKE_CURRENT_BINARY_DIR}/bingo-core")

add_subdirectory(../../bingo "${CMAKE_CURRENT_BINARY_DIR}/bingo-oracle")

