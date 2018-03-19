cmake_minimum_required(VERSION 2.8)

project(PNG C)

if(USE_SYSTEM_PNG)
    find_package(PNG)
    if (NOT PNG_FOUND)
         MESSAGE(FATAL_ERROR "Cannot find system png library")
     endif()
endif()

if (NOT PNG_FOUND)
    message(STATUS "Using local LibPNG library")
    include_directories(include)
    set(PNG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    aux_source_directory(src PNG_SOURCES)
    if (NOT ZLIB_FOUND)
        message(STATUS "Finding zlib for libpng")
        find_package(ZLIB)
    endif()
    include_directories(${ZLib_HEADERS_DIR})

    list(REMOVE_ITEM PNG_SOURCES src/pngtest.c)

    # To remove GLIBC_2.11 dependency
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0")

    add_library(png STATIC ${PNG_SOURCES})

    if (MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4267")
    endif()

    set_target_properties(png PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
    set_property(TARGET png PROPERTY FOLDER "third_party")
    if (WITH_STATIC)
        pack_static(png)
    endif()
endif()
