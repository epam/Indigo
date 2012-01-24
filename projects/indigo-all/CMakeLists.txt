cmake_minimum_required(VERSION 2.6)

project(IndigoAll C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

INCLUDE(ConfigureCommon)

add_subdirectory(../indigo "${CMAKE_CURRENT_BINARY_DIR}/indigo")
add_subdirectory(../indigo-inchi "${CMAKE_CURRENT_BINARY_DIR}/indigo-inchi")
add_subdirectory(../indigo-renderer "${CMAKE_CURRENT_BINARY_DIR}/indigo-renderer")

SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "shared/${SYSTEM_FOLDER_NAME}/${SUBSYSTEM_FOLDER_NAME}")
INCLUDE(InstallRequiredSystemLibraries)

SET(COMPONENTS static shared)
SET(CPACK_COMPONENTS_ALL ${COMPONENTS})
set(CPACK_ARCHIVE_COMPONENT_INSTALL 1)
SET(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_FILE_NAME "indigo-libs-1.1beta8-${PACKAGE_SUFFIX}") 
foreach(comp ${COMPONENTS})
	INSTALL(FILES ${Indigo_SOURCE_DIR}/LICENSE.GPL DESTINATION . COMPONENT ${comp})
	INSTALL(FILES ${Indigo_SOURCE_DIR}/indigo.h DESTINATION . COMPONENT ${comp})
	INSTALL(FILES ${Indigo_SOURCE_DIR}/renderer/indigo-renderer.h DESTINATION . COMPONENT ${comp})
	INSTALL(FILES ${Indigo_SOURCE_DIR}/plugins/inchi/indigo-inchi.h DESTINATION . COMPONENT ${comp})
endforeach()

SET(CMAKE_INSTALL_PREFIX ${Indigo_SOURCE_DIR}/dist)

INCLUDE(CPack)
