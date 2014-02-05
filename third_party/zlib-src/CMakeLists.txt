# Include this file only once)

PROJECT(ZLIB)
if (USE_SYSTEM_ZLIB)
    find_package(ZLIB)
    if (NOT ZLIB_FOUND)
         MESSAGE(FATAL_ERROR "Cannot find system zlib library")
     endif()
endif()

if (NOT ZLIB_FOUND)
    include(ConfigureCommon)
    message(STATUS "Using local ZLib library")
    file (GLOB ZLib_src src/*.c*) #src/*.def)
    file (GLOB ZLib_headers include/*.h*)
    set(ZLib_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set(ZLib_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    include_directories(${ZLib_HEADERS_DIR})

    add_library(z STATIC ${ZLib_src} ${ZLib_headers})

    if (APPLE AND NOT DEFINED $ENV{VERBOSE})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")
    endif()

    set_target_properties(z PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
    set_property(TARGET z PROPERTY FOLDER "third_party")
    if (NOT NO_STATIC)
        pack_static(z)
    endif()

    set(ZLIB_FOUND YES PARENT_SCOPE)
endif()
