cmake_minimum_required(VERSION 2.6)

project(IndigoRenderer C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

INCLUDE(ConfigureCommon)
add_subdirectory(../indigo "${CMAKE_CURRENT_BINARY_DIR}/indigo")
get_directory_property(ZLib_HEADERS_DIR DIRECTORY ../indigo DEFINITION ZLib_HEADERS_DIR)
get_directory_property(ZLIB_FOUND DIRECTORY ../indigo DEFINITION ZLIB_FOUND)
add_subdirectory(../../libpng-src "${CMAKE_CURRENT_BINARY_DIR}/png")
get_directory_property(PNG_INCLUDE_DIR DIRECTORY ../../libpng-src DEFINITION PNG_INCLUDE_DIR)
get_directory_property(PNG_LIBRARIES DIRECTORY ../../libpng-src DEFINITION PNG_LIBRARIES)
get_directory_property(PNG_FOUND DIRECTORY ../../libpng-src DEFINITION PNG_FOUND)
add_subdirectory(../../cairo-src/pixman "${CMAKE_CURRENT_BINARY_DIR}/pixman")
get_directory_property(Pixman_headers_dir DIRECTORY ../../cairo-src/pixman DEFINITION Pixman_headers_dir)
add_subdirectory(../../cairo-src/cairo "${CMAKE_CURRENT_BINARY_DIR}/cairo")
get_directory_property(Cairo_headers_dir DIRECTORY ../../cairo-src/cairo DEFINITION Cairo_headers_dir)
add_subdirectory(../../render2d "${CMAKE_CURRENT_BINARY_DIR}/render2d")
add_subdirectory(../../api/renderer "${CMAKE_CURRENT_BINARY_DIR}/renderer")

