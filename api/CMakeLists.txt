cmake_minimum_required(VERSION 2.8)

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

# Indigo static
if (NOT NO_STATIC)
	add_library(indigo STATIC ${Indigo_src} ${Indigo_headers})
	SET_TARGET_PROPERTIES(indigo PROPERTIES OUTPUT_NAME "indigo-static")
	set_target_properties(indigo PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
	target_link_libraries(indigo molecule reaction layout tinyxml)
	set_property(TARGET indigo PROPERTY FOLDER "indigo")
	# No exports in case of static library: define empty EXPORT_SYMBOL definition
	set_target_properties(indigo PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -DEXPORT_SYMBOL= ")
	pack_static(indigo)
	
	DEFINE_TEST(indigo-c-test-static "tests/c/indigo-test.c" indigo)
	# Add stdc++ library required by indigo
	SET_TARGET_PROPERTIES(indigo-c-test-static PROPERTIES LINKER_LANGUAGE CXX)
endif()

# Indigo shared
add_library(indigo-shared SHARED ${Indigo_src} ${Indigo_headers})
SET_TARGET_PROPERTIES(indigo-shared PROPERTIES OUTPUT_NAME "indigo")
set_target_properties(indigo-shared PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
target_link_libraries(indigo-shared molecule reaction layout tinyxml)
set_property(TARGET indigo-shared PROPERTY LINK_INTERFACE_LIBRARIES "")
set_property(TARGET indigo-shared PROPERTY FOLDER "indigo")
if (NOT PACK_INDIGO_NOT)
	pack_shared(indigo-shared)
endif()
DEFINE_TEST(indigo-c-test-shared "tests/c/indigo-test.c" indigo-shared)

add_executable(dlopen-test ${Indigo_SOURCE_DIR}/tests/c/dlopen-test.c)
if (UNIX)
    set_target_properties(dlopen-test PROPERTIES LINK_FLAGS "-rdynamic -ldl -pthread")
    TARGET_LINK_LIBRARIES(dlopen-test "-rdynamic -ldl -pthread")
endif()
set_property(TARGET dlopen-test PROPERTY FOLDER "tests")
#DLOPEN test
LIBRARY_NAME(indigo)
add_test(dlopen-indigo-test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dlopen-test ${Indigo_SOURCE_DIR}/libs/shared/${SYSTEM_NAME}/${SUBSYSTEM_NAME}/${indigo_NAME})