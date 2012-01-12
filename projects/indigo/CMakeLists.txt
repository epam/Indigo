cmake_minimum_required(VERSION 2.6)
#set(CMAKE_SUPPRESS_REGENERATION TRUE)

project(IndigoSolution)
if (NOT BUILD_DEBUG)
    set(CMAKE_BUILD_TYPE Release)
endif()
INCLUDE(../../common/cmake/GetSystemVersion.cmake)
INCLUDE(../../common/cmake/MakeOutputPath.cmake)

MESSAGE(STATUS "Common output dir: ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}")

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/shared)

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
add_subdirectory(../../api "${CMAKE_CURRENT_BINARY_DIR}/api")
