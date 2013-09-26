cmake_minimum_required(VERSION 2.8.8)

project(Indigo)

include(indigo-version.cmake)
include(DefineTest)

MESSAGE(STATUS "Indigo version is ${INDIGO_VERSION}")
SET(INDIGO_VERSION "${INDIGO_VERSION}" PARENT_SCOPE) # promote Indigo version
configure_file(${Indigo_SOURCE_DIR}/src/indigo_version.h.in ${Indigo_SOURCE_DIR}/src/indigo_version.h)

file (GLOB Indigo_src src/*.c*)
file (GLOB Indigo_headers *.h* src/*.h*)

# Get defined variable with the headers for TinyXML
get_directory_property(TinyXML_HEADERS_DIR DIRECTORY ../third_party/tinyxml DEFINITION TinyXML_HEADERS_DIR)

include_directories(${Indigo_SOURCE_DIR} ${Common_SOURCE_DIR} ${Common_SOURCE_DIR}/.. ${TinyXML_HEADERS_DIR})

add_library(indigoObj OBJECT ${Indigo_src} ${Indigo_headers})
set_target_properties(indigoObj PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")

#Indigo static
if (NOT NO_STATIC)
	add_library(indigo STATIC $<TARGET_OBJECTS:indigoObj> $<TARGET_OBJECTS:graph> $<TARGET_OBJECTS:layout> $<TARGET_OBJECTS:molecule> $<TARGET_OBJECTS:common> $<TARGET_OBJECTS:reaction>)
	SET_TARGET_PROPERTIES(indigo PROPERTIES OUTPUT_NAME "indigo-static")
	if (TINYXML_FOUND)
		target_link_libraries(indigo tinyxml z)
	endif()
	set_property(TARGET indigo PROPERTY FOLDER "indigo")
	# No exports in case of static library: define empty EXPORT_SYMBOL definition
	set_target_properties(indigo PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -DEXPORT_SYMBOL= ")
	pack_static(indigo)

	# DEFINE_TEST(indigo-c-test-static "tests/c/indigo-test.c;${Common_SOURCE_DIR}/hacks/memcpy.c" indigo)
	# target_link_libraries(indigo-c-test-static z tinyxml)
	# #Add stdc++ library required by indigo
	# SET_TARGET_PROPERTIES(indigo-c-test-static PROPERTIES LINKER_LANGUAGE CXX)
	# if (UNIX AND NOT APPLE)
	# 	if(${SUBSYSTEM_NAME} MATCHES "x64")
	# 		set_target_properties(indigo-c-test-static PROPERTIES LINK_FLAGS "${LINK_FLAGS} -Wl,--wrap=memcpy")
	# 	endif()
	# endif()
endif()

# Indigo shared
add_library(indigo-shared SHARED $<TARGET_OBJECTS:indigoObj> $<TARGET_OBJECTS:graph> $<TARGET_OBJECTS:layout> $<TARGET_OBJECTS:molecule> $<TARGET_OBJECTS:common> $<TARGET_OBJECTS:reaction> ${Common_SOURCE_DIR}/hacks/memcpy.c)
SET_TARGET_PROPERTIES(indigo-shared PROPERTIES OUTPUT_NAME "indigo")
set_target_properties(indigo-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
if (APPLE)
	set_target_properties(indigo-shared PROPERTIES LINK_FLAGS "${LINK_FLAGS} -undefined dynamic_lookup")
endif()
if (UNIX AND NOT APPLE)
	if(${SUBSYSTEM_NAME} MATCHES "x64")
		set_target_properties(indigo-shared PROPERTIES LINK_FLAGS "${LINK_FLAGS} -Wl,--wrap=memcpy")
	endif()
endif()
if (MSVC OR MINGW)
	set_target_properties(indigo-shared PROPERTIES PREFIX "")
endif()
target_link_libraries(indigo-shared z tinyxml)
set_property(TARGET indigo-shared PROPERTY LINK_INTERFACE_LIBRARIES "")
set_property(TARGET indigo-shared PROPERTY FOLDER "indigo")
if (NOT PACK_INDIGO_NOT)
	pack_shared(indigo-shared)
endif()
DEFINE_TEST(indigo-c-test-shared "tests/c/indigo-test.c" indigo-shared)

add_executable(dlopen-test ${Indigo_SOURCE_DIR}/tests/c/dlopen-test.c)
if (UNIX AND NOT APPLE)
  	set_target_properties(dlopen-test PROPERTIES LINK_FLAGS "-rdynamic -ldl -pthread")
    if (NOT APPLE AND ${SUBSYSTEM_NAME} MATCHES "x64")
    	set_target_properties(dlopen-test PROPERTIES LINK_FLAGS "-rdynamic -ldl -pthread -Wl,--wrap=memcpy")
    endif()
    TARGET_LINK_LIBRARIES(dlopen-test "-rdynamic -ldl -pthread")
endif()
set_property(TARGET dlopen-test PROPERTY FOLDER "tests")
#DLOPEN test
#LIBRARY_NAME(indigo)
#add_test(dlopen-indigo-test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dlopen-test ${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${indigo_NAME})
