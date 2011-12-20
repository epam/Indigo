cmake_minimum_required(VERSION 2.6)

project(PNG C)
if(USE_LOCAL_PNG)
    set(PNG_FOUND "NO")
    message(STATUS "Using local LibPNG")
else()
    find_package(PNG)
endif()
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
