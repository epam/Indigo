cmake_minimum_required(VERSION 2.6)

project(IndigoRenderer CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

file (GLOB IndigoRenderer_src src/*.c*)
file (GLOB IndigoRenderer_headers *.h src/*.h*)

include_directories(${IndigoRenderer_SOURCE_DIR} ${Indigo_SOURCE_DIR} ${Indigo_SOURCE_DIR}/src ${Cairo_SOURCE_DIR} ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)

if(MSVC)
    add_definitions(-D_WINDLL -DINDIGO_PLUGIN)
endif()
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()
add_library(indigo_renderer STATIC ${IndigoRenderer_src} ${IndigoRenderer_headers})
add_library(indigo_renderer_shared SHARED ${IndigoRenderer_src} ${IndigoRenderer_headers})
if(UNIX AND NOT APPLE)
	SET_TARGET_PROPERTIES(indigo_renderer_shared PROPERTIES LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.so.map)
endif()
if(APPLE)
	SET_TARGET_PROPERTIES(indigo_renderer_shared PROPERTIES LINK_FLAGS "-Xlinker -framework -Xlinker ApplicationServices")
endif()
target_link_libraries(indigo_renderer_shared indigo_shared cairo render2d)
