cmake_minimum_required(VERSION 2.6)

project(BingoOracle)

include(bingo-version.cmake)
include(SetBuildParameters)

message(STATUS "BingoOracle version is ${BINGO_VERSION}")
SET(BINGO_VERSION "${BINGO_VERSION}" PARENT_SCOPE) # promote Bingo version

file(GLOB Bingo_src src/oracle/*.cpp)
file(GLOB Bingo_headers src/oracle/*.h)

include_directories(src ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/.. ${OCI_INCLUDE_DIRS} ${ZLib_HEADERS_DIR})

add_library(bingo-oracle-shared SHARED ${Bingo_src} ${Bingo_headers})

set_target_properties(bingo-oracle-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

target_link_libraries(bingo-oracle-shared bingo-core molecule reaction layout common-oracle common)
if (MSVC)
	target_link_libraries(bingo-oracle-shared odbc32 odbccp32 ${OCI_LIBRARY})
elseif(APPLE)
	target_link_libraries(bingo-oracle-shared ${OCI_LIBRARY})
endif()
set_property(TARGET bingo-oracle-shared PROPERTY LINK_INTERFACE_LIBRARIES "")

SET_TARGET_PROPERTIES(bingo-oracle-shared PROPERTIES OUTPUT_NAME "bingo-oracle")

set_property(TARGET bingo-oracle-shared PROPERTY FOLDER "bingo-oracle")

pack_bingo(bingo-oracle-shared)
