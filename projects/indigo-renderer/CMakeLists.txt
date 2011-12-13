cmake_minimum_required(VERSION 2.6)

project(IndigoRenderer C CXX)

INCLUDE(../../common/cmake/GetSystemVersion.cmake)
INCLUDE(../../common/cmake/MakeOutputPath.cmake)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${COMMON_OUTPUT_PATH}/shared)
set(CMAKE_BUILD_TYPE Release)

add_subdirectory(../indigo "${CMAKE_CURRENT_BINARY_DIR}/indigo")
add_subdirectory(../../libpng-src "${CMAKE_CURRENT_BINARY_DIR}/png")
add_subdirectory(../../cairo-src/pixman "${CMAKE_CURRENT_BINARY_DIR}/pixman")
get_directory_property(Pixman_headers_dir DIRECTORY ../../cairo-src/pixman DEFINITION Pixman_headers_dir)
add_subdirectory(../../cairo-src/cairo "${CMAKE_CURRENT_BINARY_DIR}/cairo")
get_directory_property(Cairo_headers_dir DIRECTORY ../../cairo-src/cairo DEFINITION Cairo_headers_dir)
add_subdirectory(../../render2d "${CMAKE_CURRENT_BINARY_DIR}/render2d")
add_subdirectory(../../api/renderer "${CMAKE_CURRENT_BINARY_DIR}/renderer")


