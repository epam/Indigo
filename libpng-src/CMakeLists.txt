cmake_minimum_required(VERSION 2.6)

project(PNG C)

find_package(PNG)
if (NOT PNG_FOUND)
    include_directories(include)

    aux_source_directory(src PNG_SOURCES)

    find_package(ZLIB)
    if(NOT ZLIB_FOUND)
        include_directories(${ZLib_SOURCE_DIR}/include)
    endif()

    add_library(png STATIC ${PNG_SOURCES})

    target_link_libraries(png z)
endif()
