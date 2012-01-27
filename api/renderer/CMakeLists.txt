cmake_minimum_required(VERSION 2.6)

project(IndigoRenderer CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

file (GLOB IndigoRenderer_src src/*.c*)
file (GLOB IndigoRenderer_headers *.h src/*.h*)

include_directories(${IndigoRenderer_SOURCE_DIR} ${Indigo_SOURCE_DIR} ${Indigo_SOURCE_DIR}/src ${Cairo_SOURCE_DIR} ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)

if(MSVC)
    add_definitions(-D_WINDLL)
endif()
add_definitions(-DINDIGO_PLUGIN)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif()
add_library(indigo-renderer STATIC ${IndigoRenderer_src} ${IndigoRenderer_headers})
add_library(indigo-renderer-shared SHARED ${IndigoRenderer_src} ${IndigoRenderer_headers})
SET_TARGET_PROPERTIES(indigo-renderer-shared PROPERTIES OUTPUT_NAME "indigo-renderer")
if(UNIX AND NOT APPLE)
    SET_TARGET_PROPERTIES(indigo-renderer-shared PROPERTIES LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.so.map)
    SET_TARGET_PROPERTIES(indigo-renderer PROPERTIES LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.so.map)
endif()
if(APPLE)
    SET_TARGET_PROPERTIES(indigo-renderer-shared PROPERTIES LINK_FLAGS "-Xlinker -framework -Xlinker ApplicationServices -Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.explist")
    SET_TARGET_PROPERTIES(indigo-renderer PROPERTIES LINK_FLAGS "-Xlinker -framework -Xlinker ApplicationServices -Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.explist")
endif()
if(VISIBILITY_HIDDEN)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fvisibility=hidden")
endif()
if(UNIX OR APPLE)
    SET(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()
set_target_properties(indigo-renderer PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
set_target_properties(indigo-renderer-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

target_link_libraries(indigo-renderer-shared indigo-shared cairo render2d)
target_link_libraries(indigo-renderer indigo cairo render2d)

SET_TARGET_PROPERTIES(indigo-renderer PROPERTIES OUTPUT_NAME "indigo-renderer-static")

set_property(TARGET indigo-renderer-shared PROPERTY LINK_INTERFACE_LIBRARIES "")

set_property(TARGET indigo-renderer PROPERTY FOLDER "indigo-renderer")
set_property(TARGET indigo-renderer-shared PROPERTY FOLDER "indigo-renderer")

pack_shared(indigo-renderer-shared)
pack_static(indigo-renderer)

# Tests
include(DefineTest)
DEFINE_TEST(indigo-renderer-c-test-static "tests/c/indigo-renderer-test.c" indigo-renderer)
DEFINE_TEST(indigo-renderer-c-test-shared "tests/c/indigo-renderer-test.c" indigo-renderer-shared)
target_link_libraries(indigo-renderer-c-test-shared indigo-shared)
if(APPLE)
    INCLUDE(MacFrameworks)
    FIND_FRAMEWORK(ApplicationServices)
    target_link_libraries(indigo-renderer-c-test-shared ${FRAMEWORK_ApplicationServices})
    target_link_libraries(indigo-renderer-c-test-static ${FRAMEWORK_ApplicationServices})
endif()
