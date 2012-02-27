cmake_minimum_required(VERSION 2.6)

project(BingoPostgres)

include(../bingo-version.cmake)
message(STATUS "BingoPostgres version is ${BINGO_VERSION}")

find_package(POSTGRES)
if (NOT POSTGRES_FOUND)
	message(FATAL_ERROR "Postgres not found!")
endif()

file(GLOB PG_AM_src src/pg_am/*.cpp)
file(GLOB PG_AM_headers src/pg_am/*.h)
file(GLOB PG_COMMON_src src/pg_common/*.cpp)
file(GLOB PG_COMMON_headers src/pg_common/*.h)
file(GLOB PG_CORE_src src/pg_core/*.cpp)
file(GLOB PG_CORE_headers src/pg_core/*.h)

include_directories(src ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/.. ${ZLib_HEADERS_DIR}  ${BingoCoreC_HEADERS_DIR} ${POSTGRES_INCLUDE_DIRS} ${POSTGRES_INCLUDE_DIRS}/server src/pg_common/ src/pg_am/ src/pg_core/)
if (MSVC)
	include_directories(${POSTGRES_INCLUDE_DIRS}/server/port/win32_msvc ${POSTGRES_INCLUDE_DIRS}/server/port/win32)
endif()

if (NOT MSVC)
	add_library(bingo-postgres STATIC ${PG_AM_src} ${PG_AM_headers} ${PG_COMMON_src} ${PG_COMMON_headers} ${PG_CORE_src} ${PG_CORE_headers})
	add_library(bingo-postgres-shared SHARED ${PG_AM_src} ${PG_AM_headers} ${PG_COMMON_src} ${PG_COMMON_headers} ${PG_CORE_src} ${PG_CORE_headers})
else()
	add_library(bingo-postgres STATIC ${PG_AM_src} ${PG_AM_headers} ${PG_COMMON_src} ${PG_COMMON_headers} ${PG_CORE_src} ${PG_CORE_headers} ${POSTGRES_INCLUDE_DIRS}/server/port/win32.h)
	add_library(bingo-postgres-shared SHARED ${PG_AM_src} ${PG_AM_headers} ${PG_COMMON_src} ${PG_COMMON_headers} ${PG_CORE_src} ${PG_CORE_headers} ${POSTGRES_INCLUDE_DIRS}/server/port/win32.h)
endif()

set_target_properties(bingo-postgres PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
set_target_properties(bingo-postgres-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

target_link_libraries(bingo-postgres bingo-core bingo-core-c molecule reaction layout )
target_link_libraries(bingo-postgres-shared bingo-core bingo-core-c molecule reaction layout)
if (MSVC)
	target_link_libraries(bingo-postgres ${POSTGRES_LIBRARY})
	target_link_libraries(bingo-postgres-shared ${POSTGRES_LIBRARY})
endif()

set_property(TARGET bingo-postgres-shared PROPERTY LINK_INTERFACE_LIBRARIES "")

set_property(TARGET bingo-postgres PROPERTY FOLDER "bingo-postgres")
set_property(TARGET bingo-postgres-shared PROPERTY FOLDER "bingo-postgres")

#pack_shared(bingo-postgres-shared)
#pack_static(bingo-postgres)

