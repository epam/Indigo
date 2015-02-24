cmake_minimum_required(VERSION 2.8)

project(IndigoInchi C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

INCLUDE(ConfigureCommon)

add_subdirectory(../indigo "${CMAKE_CURRENT_BINARY_DIR}/indigo")
add_subdirectory(../../api/plugins/inchi "${CMAKE_CURRENT_BINARY_DIR}/indigo-inchi")
