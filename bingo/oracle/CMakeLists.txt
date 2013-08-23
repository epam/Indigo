cmake_minimum_required(VERSION 2.6)

project(BingoOracle)

include(../bingo-version.cmake)
include(ConfigureCommon)

message(STATUS "BingoOracle version is ${BINGO_VERSION_EXT}")

file(GLOB Bingo_src src/oracle/*.cpp)
file(GLOB Bingo_headers src/oracle/*.h)

include_directories(src ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/.. ${OCI_INCLUDE_DIRS} ${ZLib_HEADERS_DIR} ${BingoCore_HEADERS_DIR}
	${Common_SOURCE_DIR}/../api/plugins/inchi/src ${InChI_SOURCE_DIR}/inchi_dll)

add_library(bingo-oracle-shared SHARED ${Bingo_src} ${Bingo_headers} ${Common_SOURCE_DIR}/../api/plugins/inchi/src/indigo_inchi_core.cpp ${Common_SOURCE_DIR}/hacks/memcpy.c)

if (UNIX AND NOT APPLE)
	message(STATUS "UNIX AND NOT APPLE")
	if(${SUBSYSTEM_NAME} MATCHES "x64")
		message(STATUS "SUBSYSTEM_NAME EQUALS x64")
		set_target_properties(bingo-oracle-shared PROPERTIES LINK_FLAGS "${LINK_FLAGS} -Wl,--wrap=memcpy")
	endif()
endif()

set_target_properties(bingo-oracle-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

target_link_libraries(bingo-oracle-shared bingo-core molecule reaction layout common-oracle common inchi)
if (MSVC)
	target_link_libraries(bingo-oracle-shared odbc32 odbccp32 ${OCI_LIBRARY})
elseif(APPLE)
	target_link_libraries(bingo-oracle-shared ${OCI_LIBRARY})
endif()
set_property(TARGET bingo-oracle-shared PROPERTY LINK_INTERFACE_LIBRARIES "")

SET_TARGET_PROPERTIES(bingo-oracle-shared PROPERTIES OUTPUT_NAME "bingo-oracle")

set_property(TARGET bingo-oracle-shared PROPERTY FOLDER "bingo-oracle")

pack_bingo(bingo-oracle-shared)

