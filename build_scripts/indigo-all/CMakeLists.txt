cmake_minimum_required(VERSION 2.6)

project(IndigoAll C CXX)

ENABLE_TESTING()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

INCLUDE(ConfigureCommon)

message(STATUS "**** Indigo ****")
add_subdirectory(../indigo "${CMAKE_CURRENT_BINARY_DIR}/indigo")
message(STATUS "**** Indigo-inchi ****")
add_subdirectory(../indigo-inchi "${CMAKE_CURRENT_BINARY_DIR}/indigo-inchi")
message(STATUS "**** Indigo-renderer ****")
add_subdirectory(../indigo-renderer "${CMAKE_CURRENT_BINARY_DIR}/indigo-renderer")


SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
INCLUDE(InstallRequiredSystemLibraries)
FOREACH(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
	INSTALL(FILES ${lib} DESTINATION "shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}" COMPONENT shared)
ENDFOREACH()
  
SET(COMPONENTS static shared)

# Uncomment it in CMake 2.8.8:  http://www.gccxml.org/Bug/view.php?id=12129
SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0) 

SET(CPACK_COMPONENTS_ALL ${COMPONENTS})
set(CPACK_ARCHIVE_COMPONENT_INSTALL 1)
SET(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_FILE_NAME "indigo-libs-${INDIGO_VERSION}-${PACKAGE_SUFFIX}") 
foreach(comp ${COMPONENTS})
	INSTALL(FILES ${Indigo_SOURCE_DIR}/LICENSE.GPL DESTINATION . COMPONENT ${comp})
	INSTALL(FILES ${Indigo_SOURCE_DIR}/indigo.h DESTINATION . COMPONENT ${comp})
	INSTALL(FILES ${Indigo_SOURCE_DIR}/renderer/indigo-renderer.h DESTINATION . COMPONENT ${comp})
	INSTALL(FILES ${Indigo_SOURCE_DIR}/plugins/inchi/indigo-inchi.h DESTINATION . COMPONENT ${comp})
endforeach()

SET(CMAKE_INSTALL_PREFIX ${Indigo_SOURCE_DIR}/libs)

# Add dlopen test
if (NOT MSVC)
	ENABLE_TESTING()
	add_executable(dlopentest ${CMAKE_CURRENT_BINARY_DIR}/../../tests/api.mkv/dlopentest.c)
	ADD_TEST(dlopenTest ${CMAKE_CURRENT_BINARY_DIR}/../../tests/api.mkv/dlopentest)
endif()

INCLUDE(CPack)
INCLUDE(CTest)