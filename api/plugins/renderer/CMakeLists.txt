cmake_minimum_required(VERSION 2.8)

project(IndigoRenderer CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../common/cmake/)

file (GLOB IndigoRenderer_src src/*.c*)
file (GLOB IndigoRenderer_headers *.h src/*.h*)

include_directories(${IndigoRenderer_SOURCE_DIR} ${Indigo_SOURCE_DIR} ${Indigo_SOURCE_DIR}/src ${Cairo_SOURCE_DIR} ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/..)
include(DefineTest)

# Indigo Renderer static
if (NOT NO_STATIC)
	add_library(indigo-renderer STATIC ${IndigoRenderer_src} ${IndigoRenderer_headers})
	if(UNIX AND NOT APPLE)
		SET_TARGET_PROPERTIES(indigo-renderer PROPERTIES LINK_FLAGS -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.so.map)
	endif()
	if(APPLE)
		SET_TARGET_PROPERTIES(indigo-renderer PROPERTIES LINK_FLAGS "-Xlinker -framework -Xlinker ApplicationServices -Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.explist")
	endif()
	set_target_properties(indigo-renderer PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
	target_link_libraries(indigo-renderer indigo cairo render2d layout)
	SET_TARGET_PROPERTIES(indigo-renderer PROPERTIES OUTPUT_NAME "indigo-renderer-static")
	set_property(TARGET indigo-renderer PROPERTY FOLDER "indigo-renderer")
	# No exports in case of static library: define empty EXPORT_SYMBOL definition
	set_target_properties(indigo-renderer PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -DEXPORT_SYMBOL=")
	pack_static(indigo-renderer)
	
	DEFINE_TEST(indigo-renderer-c-test-static "tests/c/indigo-renderer-test.c" indigo-renderer)
	# Add stdc++ library required by indigo
	SET_TARGET_PROPERTIES(indigo-renderer-c-test-static PROPERTIES LINKER_LANGUAGE CXX)
	if(APPLE)
		INCLUDE(GetSystemVersion)
		INCLUDE(MacFrameworks)
		FIND_FRAMEWORK(ApplicationServices)
		target_link_libraries(indigo-renderer-c-test-static ${FRAMEWORK_ApplicationServices})
	endif()
endif()
	
# Indigo Renderer shared
add_library(indigo-renderer-shared SHARED ${IndigoRenderer_src} ${IndigoRenderer_headers})
SET_TARGET_PROPERTIES(indigo-renderer-shared PROPERTIES OUTPUT_NAME "indigo-renderer")
if(UNIX AND NOT APPLE)
    SET_TARGET_PROPERTIES(indigo-renderer-shared PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.so.map")
endif()
if(APPLE)
    SET_TARGET_PROPERTIES(indigo-renderer-shared PROPERTIES LINK_FLAGS "-undefined dynamic_lookup -Xlinker -framework -Xlinker ApplicationServices -Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/indigo_renderer.explist")    
endif()

set_target_properties(indigo-renderer-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
if (NOT APPLE)
    target_link_libraries(indigo-renderer-shared cairo render2d indigo-shared)
else()
    target_link_libraries(indigo-renderer-shared cairo render2d)
endif()
set_property(TARGET indigo-renderer-shared PROPERTY LINK_INTERFACE_LIBRARIES "")
set_property(TARGET indigo-renderer-shared PROPERTY FOLDER "indigo-renderer")

if(MSVC)
    # This should be set only for a shared library to avoid warnings
	set_target_properties(indigo-renderer-shared PROPERTIES COMPILE_FLAGS "-D_WINDLL -DINDIGO_PLUGIN")
endif()

if (NOT PACK_INDIGO_NOT)
	pack_shared(indigo-renderer-shared)
endif()
DEFINE_TEST(indigo-renderer-c-test-shared "tests/c/indigo-renderer-test.c" indigo-renderer-shared)
target_link_libraries(indigo-renderer-c-test-shared indigo-shared)
if(APPLE)
    INCLUDE(MacFrameworks)
    FIND_FRAMEWORK(ApplicationServices)
    target_link_libraries(indigo-renderer-c-test-shared ${FRAMEWORK_ApplicationServices})
endif()



# DLOPEN test
#LIBRARY_NAME(indigo-renderer)
#add_test(dlopen-indigo-renderer-test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dlopen-test ${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${indigo-renderer_NAME})
