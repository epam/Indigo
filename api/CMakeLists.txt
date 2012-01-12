cmake_minimum_required(VERSION 2.6)

project(Indigo)

file (GLOB Indigo_src src/*.c*)
file (GLOB Indigo_headers *.h* src/*.h*)

include_directories(${Indigo_SOURCE_DIR} ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()

add_library(indigo STATIC ${Indigo_src} ${Indigo_headers})
add_library(indigo_shared SHARED ${Indigo_src} ${Indigo_headers})

if(VISIBILITY_HIDDEN)
    SET_TARGET_PROPERTIES(indigo PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")
    SET_TARGET_PROPERTIES(indigo_shared PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")
endif()    

target_link_libraries(indigo molecule reaction layout)
target_link_libraries(indigo_shared molecule reaction layout)

set_property(TARGET indigo_shared PROPERTY LINK_INTERFACE_LIBRARIES "")

