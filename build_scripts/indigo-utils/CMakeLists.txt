cmake_minimum_required(VERSION 2.8)

project(IndigoUtils C CXX)
                   
SET(PACK_INDIGO_NOT YES)

if(MSVC)
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_C_LINK_FLAGS "/NODEFAULTLIB:libc.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")
	set(CMAKE_SHARED_LIBRARY_C_FLAGS "/NODEFAULTLIB:libc.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")
	set(CMAKE_CXX_LINK_FLAGS "/NODEFAULTLIB:libc.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")
	set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "/NODEFAULTLIB:libc.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrtd.lib")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

INCLUDE(ConfigureCommon)

message(STATUS "**** Indigo ****")
add_subdirectory(../indigo "${CMAKE_CURRENT_BINARY_DIR}/indigo")
message(STATUS "**** Indigo-renderer ****")
add_subdirectory(../indigo-renderer "${CMAKE_CURRENT_BINARY_DIR}/indigo-renderer")
message(STATUS "**** Indigo-cano ****")
add_subdirectory(../../utils/indigo-cano "${CMAKE_CURRENT_BINARY_DIR}/indigo-cano")
message(STATUS "**** Indigo-deco ****")
add_subdirectory(../../utils/indigo-deco "${CMAKE_CURRENT_BINARY_DIR}/indigo-deco")
message(STATUS "**** Indigo-depict ****")
add_subdirectory(../../utils/indigo-depict "${CMAKE_CURRENT_BINARY_DIR}/indigo-depict")

SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0) 

SET(CPACK_COMPONENTS_ALL "shared")
set(CPACK_ARCHIVE_COMPONENT_INSTALL 1)
SET(CPACK_GENERATOR "ZIP")
INSTALL(FILES ../../api/LICENSE.GPL DESTINATION . COMPONENT "shared")
SET(CPACK_PACKAGE_FILE_NAME "indigo-utils-${INDIGO_VERSION}-${PACKAGE_SUFFIX}")
SET(CMAKE_INSTALL_PREFIX ./libs)

INCLUDE(CPack)