cmake_minimum_required(VERSION 2.6)

project(IndigoDeco)

include(DefineTest)
include_directories(../../api ../../common)

add_executable(indigo-deco main.c)
target_link_libraries(indigo-deco indigo)
if (UNIX)
    set_target_properties(indigo-deco PROPERTIES LINK_FLAGS "-pthread")
endif()
pack_executable(indigo-deco)

add_test(NAME deco-simple-test COMMAND indigo-deco -h)
