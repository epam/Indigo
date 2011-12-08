cmake_minimum_required(VERSION 2.6)

project(Indigo)

file (GLOB Indigo_src src/*.c*)
file (GLOB Indigo_headers *.h* src/*.h*)

include_directories(${Indigo_SOURCE_DIR} ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)

add_library(indigo STATIC ${Indigo_src} ${Indigo_headers})
add_library(indigo_shared SHARED ${Indigo_src} ${Indigo_headers})

target_link_libraries(indigo molecule reaction layout)
target_link_libraries(indigo_shared molecule reaction layout)
