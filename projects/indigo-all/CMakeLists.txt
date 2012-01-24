cmake_minimum_required(VERSION 2.6)

project(IndigoAll C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

INCLUDE(ConfigureCommon)

add_subdirectory(../indigo "${CMAKE_CURRENT_BINARY_DIR}/indigo")
add_subdirectory(../indigo-inchi "${CMAKE_CURRENT_BINARY_DIR}/indigo-inchi")
add_subdirectory(../indigo-renderer "${CMAKE_CURRENT_BINARY_DIR}/indigo-renderer")

SET(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_FILE_NAME "indigo-libs-1.1beta8-${PACKAGE_SUFFIX}") 
INSTALL(FILES ${Indigo_SOURCE_DIR}/LICENSE.GPL DESTINATION .)
INSTALL(FILES ${Indigo_SOURCE_DIR}/indigo.h DESTINATION .)
INSTALL(FILES ${Indigo_SOURCE_DIR}/renderer/indigo-renderer.h DESTINATION .)
INSTALL(FILES ${Indigo_SOURCE_DIR}/plugins/inchi/indigo-inchi.h DESTINATION .)

INCLUDE(CPack)
