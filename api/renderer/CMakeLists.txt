cmake_minimum_required(VERSION 2.6)

project(IndigoRenderer CXX)

file (GLOB IndigoRenderer_src src/*.c*)
file (GLOB IndigoRenderer_headers *.h src/*.h*)

include_directories(${IndigoRenderer_SOURCE_DIR} ${Indigo_SOURCE_DIR} ${Indigo_SOURCE_DIR}/src ${Cairo_SOURCE_DIR} ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)

add_library(indigo_renderer STATIC ${IndigoRenderer_src} ${IndigoRenderer_headers})
add_library(indigo_renderer_shared SHARED ${IndigoRenderer_src} ${IndigoRenderer_headers})

target_link_libraries(indigo_renderer indigo cairo)
target_link_libraries(indigo_renderer_shared indigo cairo)