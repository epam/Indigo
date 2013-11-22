cmake_minimum_required(VERSION 2.6)

project(IndigoDepict)

include(DefineTest)
include_directories(../../api ../../api/plugins/renderer ../../common)

add_executable(indigo-depict main.c)
if(APPLE)
	INCLUDE(GetSystemVersion)
	INCLUDE(MacFrameworks)
	FIND_FRAMEWORK(ApplicationServices)
	target_link_libraries(indigo-depict ${FRAMEWORK_ApplicationServices})
	SET_TARGET_PROPERTIES(indigo-depict PROPERTIES LINK_FLAGS "${LINK_FLAGS} -Xlinker -framework -Xlinker ApplicationServices")
endif()
target_link_libraries(indigo-depict indigo indigo-renderer)
if (UNIX)
    set_target_properties(indigo-depict PROPERTIES LINK_FLAGS "-pthread")
endif()
pack_executable(indigo-depict)

add_test(NAME depict-simple-test COMMAND indigo-depict - "[CH2:1]=[CH:2][CH2:1][CH2:3][C:4](C)[CH2:3]" output.png)
add_test(NAME depict-query-test COMMAND indigo-depict - "CC.[O-][*-]([O-])=O" query.png -query)