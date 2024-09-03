cmake_minimum_required(VERSION 3.5)

project(lunasvg VERSION 2.4.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

add_library(lunasvg)

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(3rdparty/plutovg)

target_include_directories(lunasvg
        INTERFACE 3rdparty/stb
)

target_compile_definitions(lunasvg PRIVATE LUNASVG_BUILD)
target_compile_definitions(lunasvg PUBLIC LUNASVG_BUILD_STATIC)
