cmake_minimum_required(VERSION 2.6)

project(IndigoRenderer CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

file (GLOB IndigoRenderer_src src/*.c*)
file (GLOB IndigoRenderer_headers *.h src/*.h*)

include_directories(${IndigoRenderer_SOURCE_DIR} ${Indigo_SOURCE_DIR} ${Indigo_SOURCE_DIR}/src ${Cairo_SOURCE_DIR} ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)

if(MSVC)
    add_definitions(-D_WINDLL -DINDIGO_PLUGIN)
endif()
add_library(indigo_renderer STATIC ${IndigoRenderer_src} ${IndigoRenderer_headers})
add_library(indigo_renderer_shared SHARED ${IndigoRenderer_src} ${IndigoRenderer_headers})
if (MSVC)
    target_link_libraries(indigo_renderer_shared indigo_shared cairo render2d)
else()
    target_link_libraries(indigo_renderer_shared indigo_shared cairo render2d)
endif()
