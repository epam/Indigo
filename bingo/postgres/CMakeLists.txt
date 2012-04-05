cmake_minimum_required(VERSION 2.8)

project(BingoPostgres)

include(../bingo-version.cmake)
include(SetBuildParameters)
SET(BINGO_VERSION "${BINGO_VERSION}" PARENT_SCOPE) # promote Bingo version

message(STATUS "BingoPostgres version is ${BINGO_VERSION}")

find_package(POSTGRES)
if (NOT POSTGRES_FOUND)
	message(FATAL_ERROR "Postgres not found!")
endif()
message(STATUS "Postgres found at ${POSTGRES_INCLUDE_DIRS}")

file(GLOB PG_AM_src src/pg_am/*.cpp src/pg_am/*.c)
file(GLOB PG_AM_headers src/pg_am/*.h)
file(GLOB PG_COMMON_src src/pg_common/*.cpp src/pg_common/*.c)
file(GLOB PG_COMMON_headers src/pg_common/*.h)
file(GLOB PG_CORE_src src/pg_core/*.cpp src/pg_core/*.c)
file(GLOB PG_CORE_headers src/pg_core/*.h)
file(GLOB BingoCore_src ../src/core/*.cpp ../src/core/*.c)
file(GLOB BingoCore_headers ../src/core/*.h)
file(GLOB BingoCoreC_src ../src/core-c/*.cpp ../src/core-c/*.c)
file(GLOB BingoCoreC_headers ../src/core-c/*.h)

include_directories(src ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/.. ${ZLib_HEADERS_DIR}  ${BingoCoreC_HEADERS_DIR} ${BingoCore_HEADERS_DIR} ${POSTGRES_INCLUDE_DIRS} src/pg_common/ src/pg_am/ src/pg_core/ ../src ../src/core ../src/core-c)
if (MSVC)
	include_directories(${POSTGRES_INCLUDE_DIRS}/port/win32_msvc ${POSTGRES_INCLUDE_DIRS}/port/win32 ${POSTGRES_INCLUDE_DIRS}/..)
endif()

if (NOT MSVC)                                                                                                                                                            
#	add_library(bingo-postgres-shared SHARED ${PG_AM_src} ${PG_AM_headers} ${PG_COMMON_src} ${PG_COMMON_headers} ${PG_CORE_src} ${PG_CORE_headers} ${BingoCore_src} ${BingoCore_headers} ${BingoCoreC_src} ${BingoCoreC_headers})
	add_library(bingo-postgres-shared SHARED ${PG_AM_src} ${PG_AM_headers} ${PG_COMMON_src} ${PG_COMMON_headers} ${PG_CORE_src} ${PG_CORE_headers} ${BingoCore_src} ${BingoCore_headers})
else()
#	add_library(bingo-postgres-shared SHARED ${PG_AM_src} ${PG_AM_headers} ${PG_COMMON_src} ${PG_COMMON_headers} ${PG_CORE_src} ${PG_CORE_headers} ${POSTGRES_INCLUDE_DIRS}/port/win32.h ${BingoCore_src} ${BingoCore_headers} ${BingoCoreC_src} ${BingoCoreC_headers})
	add_library(bingo-postgres-shared SHARED ${PG_AM_src} ${PG_AM_headers} ${PG_COMMON_src} ${PG_COMMON_headers} ${PG_CORE_src} ${PG_CORE_headers} ${POSTGRES_INCLUDE_DIRS}/port/win32.h)
endif()
SET_TARGET_PROPERTIES(bingo-postgres-shared PROPERTIES OUTPUT_NAME "bingo_postgres")
SET_TARGET_PROPERTIES(bingo-postgres-shared PROPERTIES PREFIX "")

set_target_properties(bingo-postgres-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
if (APPLE)
	set_target_properties(bingo-postgres-shared PROPERTIES LINK_FLAGS "${LINK_FLAGS} -undefined suppress -flat_namespace")
elseif(UNIX)
	set_target_properties(bingo-postgres-shared PROPERTIES LINK_FLAGS "${LINK_FLAGS} -pthread")
endif()

target_link_libraries(bingo-postgres-shared molecule reaction layout bingo-core bingo-core-c)
if (MSVC)
	target_link_libraries(bingo-postgres-shared ${POSTGRES_LIBRARY})
endif()

set_property(TARGET bingo-postgres-shared PROPERTY LINK_INTERFACE_LIBRARIES "")
set_property(TARGET bingo-postgres-shared PROPERTY FOLDER "bingo-postgres")

pack_bingo(bingo-postgres-shared)

