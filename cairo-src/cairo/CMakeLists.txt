cmake_minimum_required(VERSION 2.6)

project(Cairo C)

set(Cairo_headers_dir ${CMAKE_CURRENT_SOURCE_DIR}/src/)

include_directories(${Cairo_headers_dir} ${Pixman_headers_dir})

add_definitions(-DHAVE_STDINT_H -DHAVE_PTHREAD_H -DCAIRO_HAS_DIRECTFB_SURFACE)

file (GLOB CAIRO_SOURCES src/*.c)
file (GLOB CAIRO_HEADERS src/*.h)

list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-xlib-screen.c)
list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-xlib-surface.c)
list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-xlib-visual.c)
list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-xlib-display.c)
list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-xcb-surface.c)
list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-os2-surface.c)
list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/check-has-hidden-symbols.c)
list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-glitz-surface.c)
list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-directfb-surface.c)
if(NOT MSVC)
    list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-win32-surface.c)
    list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-win32-printing-surface.c)
    list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-win32-font-surface.c)
    list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-win32-font.c)
endif(NOT MSVC)
if (NOT APPLE)
    list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-quartz-image-surface.c)
    list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-quartz-font.c)
    list(REMOVE_ITEM CAIRO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-quartz-surface.c)
endif(NOT APPLE)

if(UNIX)
    find_package(Freetype REQUIRED)
    if(NOT FREETYPE_FOUND)
        message(FATAL_ERROR "No Freetype found")  
    endif()
    add_definitions(-DCAIRO_HAS_FT_FONT)
    include_directories(${FREETYPE_INCLUDE_DIRS})
endif(UNIX)
find_package(PNG)
if (NOT PNG_FOUND)
    include_directories(${PNG_SOURCE_DIR}/include)
endif()


add_library(cairo STATIC ${CAIRO_SOURCES})
    target_link_libraries(cairo pixman)
if (NOT PNG_FOUND)
    target_link_library(cairo png)
endif()

