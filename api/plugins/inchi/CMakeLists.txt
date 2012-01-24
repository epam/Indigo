cmake_minimum_required(VERSION 2.6)

project(IndigoInchi CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/cmake/)

file (GLOB IndigoInchi_src src/*.c*)
file (GLOB IndigoInchi_headers *.h src/*.h*)

include_directories(${IndigoInchi_SOURCE_DIR} 
	${Indigo_SOURCE_DIR}
	${Indigo_SOURCE_DIR}/src
	${Common_SOURCE_DIR}
	${Common_SOURCE_DIR}/..
	${InChI_SOURCE_DIR}/inchi_dll)

if(MSVC)
    add_definitions(-D_WINDLL -DINDIGO_PLUGIN)
endif()
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()
add_library(indigo-inchi STATIC ${IndigoInchi_src} ${IndigoInchi_headers})
add_library(indigo-inchi-shared SHARED ${IndigoInchi_src} ${IndigoInchi_headers})
SET_TARGET_PROPERTIES(indigo-inchi-shared PROPERTIES OUTPUT_NAME "indigo-inchi")
if(UNIX AND NOT APPLE)
	SET_TARGET_PROPERTIES(indigo-inchi-shared PROPERTIES
		LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/indigo-inchi.so.map)
	SET_TARGET_PROPERTIES(indigo-inchi PROPERTIES
		LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/indigo-inchi.so.map)
endif()
if(APPLE)
	SET_TARGET_PROPERTIES(indigo-inchi-shared PROPERTIES LINK_FLAGS "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.explist")
	SET_TARGET_PROPERTIES(indigo-inchi PROPERTIES LINK_FLAGS "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.explist")
endif()
if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()
set_target_properties(indigo-inchi PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
set_target_properties(indigo-inchi-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
target_link_libraries(indigo-inchi-shared indigo-shared inchi)
