cmake_minimum_required(VERSION 2.6)

project(Indigo)

file (GLOB Indigo_src src/*.c*)
file (GLOB Indigo_headers *.h* src/*.h*)

include_directories(${Indigo_SOURCE_DIR} ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()

add_library(indigo STATIC ${Indigo_src} ${Indigo_headers})
add_library(indigo-shared SHARED ${Indigo_src} ${Indigo_headers})
SET_TARGET_PROPERTIES(indigo PROPERTIES OUTPUT_NAME "indigo-static")
SET_TARGET_PROPERTIES(indigo-shared PROPERTIES OUTPUT_NAME "indigo")

if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")

endif()    
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()

set_target_properties(indigo PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
set_target_properties(indigo-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

target_link_libraries(indigo molecule reaction layout)
target_link_libraries(indigo-shared molecule reaction layout)

set_property(TARGET indigo-shared PROPERTY LINK_INTERFACE_LIBRARIES "")

set_property(TARGET indigo PROPERTY FOLDER "indigo")
set_property(TARGET indigo-shared PROPERTY FOLDER "indigo")

INSTALL(TARGETS indigo-shared
		  RUNTIME DESTINATION shared)
INSTALL(TARGETS indigo DESTINATION static)
		  