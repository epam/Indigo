cmake_minimum_required(VERSION 2.6)

project(IndigoRenderer C CXX)

add_subdirectory(../indigo "${CMAKE_CURRENT_BINARY_DIR}/indigo")
add_subdirectory(../../cairo-src/pixman "${CMAKE_CURRENT_BINARY_DIR}/pixman")
add_subdirectory(../../libpng-src "${CMAKE_CURRENT_BINARY_DIR}/png")
get_directory_property(Pixman_headers_dir DIRECTORY ../../cairo-src/pixman DEFINITION Pixman_headers_dir)
message(STATUS ${Pixman_headers_dir})
add_subdirectory(../../cairo-src/cairo "${CMAKE_CURRENT_BINARY_DIR}/cairo")
add_subdirectory(../../api/renderer "${CMAKE_CURRENT_BINARY_DIR}/renderer")
