cmake_minimum_required(VERSION 2.8)

project(IndigoUtils C CXX)
                   
SET(PACK_INDIGO_NOT YES)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

INCLUDE(ConfigureCommon)

message(STATUS "**** Indigo-cano ****")
add_subdirectory(../indigo-cano "${CMAKE_CURRENT_BINARY_DIR}/indigo-cano")
message(STATUS "**** Indigo-deco ****")
add_subdirectory(../indigo-deco "${CMAKE_CURRENT_BINARY_DIR}/indigo-deco")
message(STATUS "**** Indigo-depict ****")
add_subdirectory(../indigo-depict "${CMAKE_CURRENT_BINARY_DIR}/indigo-depict")

SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0) 

SET(CPACK_COMPONENTS_ALL "shared")
set(CPACK_ARCHIVE_COMPONENT_INSTALL 1)
SET(CPACK_GENERATOR "ZIP")
INSTALL(FILES ../../api/LICENSE.GPL DESTINATION . COMPONENT "shared")
SET(CPACK_PACKAGE_FILE_NAME "indigo-utils-${INDIGO_VERSION}-${PACKAGE_SUFFIX}")
SET(CMAKE_INSTALL_PREFIX ./libs)

INCLUDE(CPack)