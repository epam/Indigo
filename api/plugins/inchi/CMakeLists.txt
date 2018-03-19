cmake_minimum_required(VERSION 2.8)

project(IndigoInchi CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/cmake/)

file (GLOB IndigoInchi_src src/*.c*)
file (GLOB IndigoInchi_headers *.h src/*.h*)

include_directories(${IndigoInchi_SOURCE_DIR}
	${Indigo_SOURCE_DIR}
	${Indigo_SOURCE_DIR}/src
	${Common_SOURCE_DIR}
	${Common_SOURCE_DIR}/..
	${InChI_SOURCE_DIR}/INCHI_BASE/src)
include(DefineTest)

add_library(indigo-inchi-Obj OBJECT ${IndigoInchi_src} ${IndigoInchi_headers})
set_target_properties(indigo-inchi-Obj PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

# Indigo InChI static
if (WITH_STATIC)
	add_library(indigo-inchi STATIC $<TARGET_OBJECTS:indigo-inchi-Obj>)
	if(UNIX AND NOT APPLE)
		SET_TARGET_PROPERTIES(indigo-inchi PROPERTIES LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/indigo_inchi.so.map)
	endif()
	if(APPLE)
		SET_TARGET_PROPERTIES(indigo-inchi PROPERTIES LINK_FLAGS "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/indigo_inchi.explist")
	endif()
	set_target_properties(indigo-inchi PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
	SET_TARGET_PROPERTIES(indigo-inchi PROPERTIES OUTPUT_NAME "indigo-inchi-static")
	set_property(TARGET indigo-inchi PROPERTY FOLDER "indigo-inchi")
	# No exports in case of static library: define empty EXPORT_SYMBOL definition
	set_target_properties(indigo-inchi PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -DEXPORT_SYMBOL=")
	PACK_STATIC(indigo-inchi)

	# DEFINE_TEST(indigo-inchi-c-test-static "tests/c/indigo-inchi-test.c;${Common_SOURCE_DIR}/hacks/memcpy.c" indigo-inchi)
	# target_link_libraries(indigo-inchi-c-test-static inchi)
	# #Add stdc++ library required by indigo
	# SET_TARGET_PROPERTIES(indigo-inchi-c-test-static PROPERTIES LINKER_LANGUAGE CXX)
	# if (UNIX AND NOT APPLE)
	# 	if(${SUBSYSTEM_NAME} MATCHES "x64")
	# 		set_target_properties(indigo-inchi-c-test-static PROPERTIES LINK_FLAGS "${LINK_FLAGS} -Wl,--wrap=memcpy")
	# 		endif()
	# endif()
endif()

# Indigo InChI shared
add_library(indigo-inchi-shared SHARED $<TARGET_OBJECTS:indigo-inchi-Obj> ${Common_SOURCE_DIR}/hacks/memcpy.c)
target_link_libraries(indigo-inchi-shared inchi)
SET_TARGET_PROPERTIES(indigo-inchi-shared PROPERTIES OUTPUT_NAME "indigo-inchi")
if (MSVC OR MINGW)
	set_target_properties(indigo-inchi-shared PROPERTIES PREFIX "")
endif()
if(UNIX AND NOT APPLE)
	SET_TARGET_PROPERTIES(indigo-inchi-shared PROPERTIES
		LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/indigo_inchi.so.map)
endif()
if(APPLE)
	SET_TARGET_PROPERTIES(indigo-inchi-shared PROPERTIES LINK_FLAGS "-undefined dynamic_lookup  -Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/indigo_inchi.explist")
endif()
set_target_properties(indigo-inchi-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
if (UNIX AND NOT APPLE)
	if(${SUBSYSTEM_NAME} MATCHES "x64")
		set_target_properties(indigo-inchi-shared PROPERTIES LINK_FLAGS "${LINK_FLAGS} -Wl,--wrap=memcpy")
		endif()
endif()

if(MSVC)
    # This should be set only for a shared library to avoid warnings
	set_target_properties(indigo-inchi-shared PROPERTIES COMPILE_FLAGS "-D_WINDLL -DINDIGO_PLUGIN")
endif()

if (MSVC OR MINGW)
	target_link_libraries(indigo-inchi-shared indigo-shared)
	set_property(TARGET indigo-inchi-shared PROPERTY LINK_INTERFACE_LIBRARIES "inchi;indigo-shared")
else()
	set_property(TARGET indigo-inchi-shared PROPERTY LINK_INTERFACE_LIBRARIES "inchi")
endif()
set_property(TARGET indigo-inchi-shared PROPERTY FOLDER "indigo-inchi")
IF (NOT PACK_INDIGO_NOT)
	PACK_SHARED(indigo-inchi-shared)
ENDIF()
DEFINE_TEST(indigo-inchi-c-test-shared "tests/c/indigo-inchi-test.c" "")
ADD_DEPENDENCIES(indigo-inchi-c-test-shared indigo-inchi-shared)
target_link_libraries(indigo-inchi-c-test-shared indigo-shared indigo-inchi-shared)
if (UNIX AND NOT APPLE)
	target_link_libraries(indigo-inchi-c-test-shared pthread)
endif()
