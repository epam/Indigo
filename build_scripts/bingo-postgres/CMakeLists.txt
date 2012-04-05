cmake_minimum_required(VERSION 2.8)

project(BingoPostgresSolution)

set(BINGO "YES")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)
set(Bingo_SOURCE_DIR ../../bingo/)
set(BingoPostgres_SOURCE_DIR ../../bingo/postgres)

INCLUDE(ConfigureCommon)

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

add_subdirectory(../../zlib-src "${CMAKE_CURRENT_BINARY_DIR}/zlib")
# Get defined variable with the headers
get_directory_property(ZLib_HEADERS_DIR DIRECTORY ../../zlib-src DEFINITION ZLib_HEADERS_DIR)

add_subdirectory(../../common "${CMAKE_CURRENT_BINARY_DIR}/common")

add_subdirectory(../../tinyxml "${CMAKE_CURRENT_BINARY_DIR}/tinyxml")
# Get defined variable with the headers
get_directory_property(TinyXML_HEADERS_DIR DIRECTORY ../../tinyxml DEFINITION TinyXML_HEADERS_DIR)

add_subdirectory(../../graph "${CMAKE_CURRENT_BINARY_DIR}/graph")
add_subdirectory(../../molecule "${CMAKE_CURRENT_BINARY_DIR}/molecule")
add_subdirectory(../../reaction "${CMAKE_CURRENT_BINARY_DIR}/reaction")

add_subdirectory(../../layout "${CMAKE_CURRENT_BINARY_DIR}/layout")

add_subdirectory(../../bingo/bingo-core "${CMAKE_CURRENT_BINARY_DIR}/bingo-core")
get_directory_property(BingoCore_HEADERS_DIR DIRECTORY ../../bingo/bingo-core DEFINITION BingoCore_HEADERS_DIR)

add_subdirectory(../../bingo/bingo-core-c "${CMAKE_CURRENT_BINARY_DIR}/bingo-core-c")
get_directory_property(BingoCoreC_HEADERS_DIR DIRECTORY ../../bingo/bingo-core-c DEFINITION BingoCoreC_HEADERS_DIR)

add_subdirectory(../../bingo/postgres "${CMAKE_CURRENT_BINARY_DIR}/bingo-postgres")

SET(BINGO_VERSION "${BINGO_VERSION}")

SET(COMPONENTS shared)

# Uncomment it in CMake 2.8.8:  http://www.gccxml.org/Bug/view.php?id=12129
SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)

SET(CPACK_COMPONENTS_ALL ${COMPONENTS})
set(CPACK_ARCHIVE_COMPONENT_INSTALL 1)
SET(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_FILE_NAME "bingo-postgres$ENV{BINGO_PG_VERSION}-${BINGO_VERSION}-${PACKAGE_SUFFIX}")
foreach(comp ${COMPONENTS})
	INSTALL(FILES ${Bingo_SOURCE_DIR}/LICENSE.GPL DESTINATION . COMPONENT ${comp})
	INSTALL(FILES ${BingoPostgres_SOURCE_DIR}/INSTALL DESTINATION . COMPONENT ${comp})
	INSTALL(FILES ${BingoPostgres_SOURCE_DIR}/README DESTINATION . COMPONENT ${comp})
	INSTALL(DIRECTORY ${BingoPostgres_SOURCE_DIR}/sql/$ENV{BINGO_PG_VERSION}/ DESTINATION sql COMPONENT ${comp})
	IF(${SYSTEM_NAME} MATCHES "Win")
		INSTALL(FILES ${BingoPostgres_SOURCE_DIR}/bingo-pg-install.bat DESTINATION . COMPONENT ${comp})
	ELSE()
		INSTALL(FILES ${BingoPostgres_SOURCE_DIR}/bingo-pg-install.sh DESTINATION . COMPONENT ${comp})
	ENDIF()
endforeach()

SET(CMAKE_INSTALL_PREFIX ${BingoPostgres_SOURCE_DIR}/libs)

INCLUDE(CPack)
