cmake_minimum_required(VERSION 2.6)

project(BingoOracle)

include(bingo-version.cmake)
message(STATUS "BingoOracle version is ${BINGO_VERSION}")

file(GLOB Bingo_src src/oracle/*.cpp)
file(GLOB Bingo_headers src/oracle/*.h)

include_directories(src ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..})

add_library(bingo-oracle STATIC ${Bingo_src} ${Bingo_headers})
add_library(bingo-oracle-shared SHARED ${Bingo_src} ${Bingo_headers})

find_package(OCI)
IF(OCI_FOUND)
	message(STATUS "OCI found at ${OCI_LIBRARY_DIRS}")
ELSE()
	message(FATAL_ERROR "OCI not found!")
ENDIF()
target_link_libraries(bingo-oracle bingo-core-c bingo-core molecule reaction layout oci)
target_link_libraries(bingo-oracle-shared bingo-core-c bingo-core molecule reaction layout oci)

set_property(TARGET bingo-oracle-shared PROPERTY LINK_INTERFACE_LIBRARIES "")

set_property(TARGET bingo-oracle PROPERTY FOLDER "bingo-oracle")
set_property(TARGET bingo-oracle-shared PROPERTY FOLDER "bingo-oracle")

#pack_shared(bingo-oracle-shared)
#pack_static(bingo-oracle)

