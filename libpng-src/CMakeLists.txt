cmake_minimum_required(VERSION 2.6)

project(PNG C)

include_directories(include)
aux_source_directory(src PNG_SOURCES)
add_library(png STATIC ${PNG_SOURCES})
