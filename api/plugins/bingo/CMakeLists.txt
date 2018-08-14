cmake_minimum_required(VERSION 2.8)

project(Bingo CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/cmake/)

file (GLOB Bingo_src src/*.c*)
file (GLOB Bingo_headers *.h src/*.h*)

include(ConfigureCommon)

include_directories(${Bingo_SOURCE_DIR}
	${Indigo_SOURCE_DIR}
	${Indigo_SOURCE_DIR}/src
	${Common_SOURCE_DIR}
	${Common_SOURCE_DIR}/..)
include(DefineTest)

# Bingo static
if (WITH_STATIC)
	add_library(bingo STATIC ${Bingo_src} ${Bingo_headers})
	if(UNIX AND NOT APPLE)
		SET_TARGET_PROPERTIES(bingo PROPERTIES LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/bingo.so.map)
	endif()
	if(APPLE)
		SET_TARGET_PROPERTIES(bingo PROPERTIES LINK_FLAGS "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/bingo.explist")
	endif()
	set_target_properties(bingo PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
	SET_TARGET_PROPERTIES(bingo PROPERTIES OUTPUT_NAME "bingo-static")
	set_property(TARGET bingo PROPERTY FOLDER "bingo")
	# No exports in case of static library: define empty EXPORT_SYMBOL definition
	set_target_properties(bingo PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -DEXPORT_SYMBOL=")
	PACK_STATIC(bingo)

	#DEFINE_TEST(bingo-test-static "tests/c/bingo-test.c" bingo indigo)
	# Add stdc++ library required by indigo
	#SET_TARGET_PROPERTIES(bingo-test-static PROPERTIES LINKER_LANGUAGE CXX)
endif()

# Bingo shared
add_library(bingo-shared SHARED ${Bingo_src} ${Bingo_headers} ${Common_SOURCE_DIR}/hacks/memcpy.c)
if (MSVC OR MINGW)
	set_target_properties(bingo-shared PROPERTIES PREFIX "")
endif()
if (UNIX AND NOT APPLE)
	if(${SUBSYSTEM_NAME} MATCHES "x64")
		set(LINK_FLAGS "${LINK_FLAGS} -Wl,--wrap=memcpy")
	endif()
endif()

SET_TARGET_PROPERTIES(bingo-shared PROPERTIES OUTPUT_NAME "bingo")
if(UNIX AND NOT APPLE)
	 SET_TARGET_PROPERTIES(bingo-shared PROPERTIES LINK_FLAGS "${LINK_FLAGS} -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/bingo.so.map")
endif()
if(APPLE)
	SET_TARGET_PROPERTIES(bingo-shared PROPERTIES LINK_FLAGS "${LINK_FLAGS} -undefined dynamic_lookup  -Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/bingo.explist")
endif()
set_target_properties(bingo-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

if(MSVC)
	# This should be set only for a shared library to avoid warnings
	set_target_properties(bingo-shared PROPERTIES COMPILE_FLAGS "-D_WINDLL -DINDIGO_PLUGIN")
endif()

if (MSVC OR MINGW)
	target_link_libraries(bingo-shared indigo-shared)
endif()
set_property(TARGET bingo-shared PROPERTY LINK_INTERFACE_LIBRARIES "")
set_property(TARGET bingo-shared PROPERTY FOLDER "bingo")
IF (NOT PACK_INDIGO_NOT)
	PACK_SHARED(bingo-shared)
ENDIF()

#DEFINE_TEST(bingo-test-shared "tests/c/bingo-test.c" bingo-shared indigo-shared)
# Add stdc++ library required by indigo
#SET_TARGET_PROPERTIES(bingo-test-shared PROPERTIES LINKER_LANGUAGE CXX)
