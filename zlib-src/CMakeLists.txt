PROJECT(z)
if(USE_LOCAL_ZLIB)
    set(ZLIB_FOUND "NO")
    message(STATUS "Using local ZLib")
else()
    find_package(ZLIB)
endif()
if (NOT ZLIB_FOUND)
    file (GLOB ZLib_src src/*.c* src/*.def)
    file (GLOB ZLib_headers include/*.h*)

    set(ZLib_HEADERS_DIR ${ZLib_SOURCE_DIR}/include)

    include_directories(${ZLib_HEADERS_DIR})

    add_library(z ${ZLib_src} ${ZLib_headers})
endif()
