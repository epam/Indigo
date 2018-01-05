cmake_minimum_required(VERSION 2.8)

project(IndigoAll C CXX)

if (NOT DEFINED ENV{DISABLE_INDIGO_TESTS})
	ENABLE_TESTING()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

INCLUDE(ConfigureCommon)

message(STATUS "**** Indigo ****")
add_subdirectory(../indigo "${CMAKE_CURRENT_BINARY_DIR}/indigo")
message(STATUS "**** Indigo-inchi ****")
add_subdirectory(../indigo-inchi "${CMAKE_CURRENT_BINARY_DIR}/indigo-inchi")
message(STATUS "**** Indigo-renderer ****")
add_subdirectory(../indigo-renderer "${CMAKE_CURRENT_BINARY_DIR}/indigo-renderer")
message(STATUS "**** Bingo ****")
add_subdirectory(../bingo "${CMAKE_CURRENT_BINARY_DIR}/bingo")

SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
INCLUDE(InstallRequiredSystemLibraries)
FOREACH(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
    INSTALL(FILES ${lib} DESTINATION "shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}" COMPONENT shared)
ENDFOREACH()

if (NOT NO_STATIC)
    set(COMPONENTS static shared)
else()
    set(COMPONENTS shared)
endif()

# Uncomment it in CMake 2.8.8:  http://www.gccxml.org/Bug/view.php?id=12129
SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

SET(CPACK_COMPONENTS_ALL ${COMPONENTS})
set(CPACK_ARCHIVE_COMPONENT_INSTALL 1)
SET(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_FILE_NAME "indigo-libs-${INDIGO_VERSION}-${PACKAGE_SUFFIX}")

foreach(comp ${COMPONENTS})
    INSTALL(FILES ${Indigo_SOURCE_DIR}/LICENSE.GPL DESTINATION . COMPONENT ${comp})
    INSTALL(FILES ${Indigo_SOURCE_DIR}/indigo.h DESTINATION . COMPONENT ${comp})
    INSTALL(FILES ${Indigo_SOURCE_DIR}/plugins/renderer/indigo-renderer.h DESTINATION . COMPONENT ${comp})
    INSTALL(FILES ${Indigo_SOURCE_DIR}/plugins/inchi/indigo-inchi.h DESTINATION . COMPONENT ${comp})
    INSTALL(FILES ${Indigo_SOURCE_DIR}/plugins/bingo/bingo.h DESTINATION . COMPONENT ${comp})
endforeach()

SET(CMAKE_INSTALL_PREFIX ${Indigo_SOURCE_DIR}/libs)

# DLOPEN TEST
LIBRARY_NAME(indigo)
LIBRARY_NAME(indigo-inchi)
LIBRARY_NAME(indigo-renderer)
LIBRARY_NAME(bingo)

if (NOT DEFINED ENV{DISABLE_INDIGO_TESTS})
add_test(dlopen-test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dlopen-test ${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${indigo_NAME} ${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${indigo-inchi_NAME} ${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${indigo-renderer_NAME} ${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${bingo_NAME})
endif()

if(UNIX AND NOT APPLE AND CHECK_ABI)
add_test(abi-test-indigo "file" "${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${indigo_NAME}")
set_tests_properties (abi-test-indigo PROPERTIES PASS_REGULAR_EXPRESSION "SYSV")
add_test(abi-test-indigo-inchi "file" "${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${indigo-inchi_NAME}")
set_tests_properties (abi-test-indigo-inchi PROPERTIES PASS_REGULAR_EXPRESSION "SYSV")
add_test(abi-test-indigo-renderer "file" "${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${indigo-renderer_NAME}")
set_tests_properties (abi-test-indigo-renderer PROPERTIES PASS_REGULAR_EXPRESSION "SYSV")
add_test(abi-test-bingo "file" "${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${bingo_NAME}")
set_tests_properties (abi-test-bingo PROPERTIES PASS_REGULAR_EXPRESSION "SYSV")
endif()

INCLUDE(CPack)
INCLUDE(CTest)

