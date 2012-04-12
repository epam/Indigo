cmake_minimum_required(VERSION 2.6)

project(IndigoCano)

include_directories(../../api)

add_executable(indigo-cano main.c)
target_link_libraries(indigo-cano indigo)
if (UNIX)
    set_target_properties(indigo-cano PROPERTIES LINK_FLAGS "-pthread")
endif()
pack_executable(indigo-cano)            
