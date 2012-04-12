cmake_minimum_required(VERSION 2.6)

project(IndigoDepict)

include_directories(../../api ../../api/renderer ../../common)

add_executable(indigo-depict main.c)
target_link_libraries(indigo-depict indigo indigo-renderer)
if (UNIX)
    set_target_properties(indigo-depict PROPERTIES LINK_FLAGS "-pthread")
endif()
pack_executable(indigo-depict)            
