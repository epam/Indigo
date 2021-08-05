cmake_minimum_required(VERSION 3.4)

project(cairo C)

file(GLOB ${PROJECT_NAME}_SOURCES
        src/*.c
        boilerplate/*.c
)

if (WIN32)
    file(GLOB ${PROJECT_NAME}_SOURCES_WIN32
            src/win32/*.c
    )
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_SOURCES_WIN32})
target_include_directories(${PROJECT_NAME}
        PUBLIC src
        PRIVATE boilerplate
)
target_link_libraries(${PROJECT_NAME}
        PUBLIC pixman
        PUBLIC png)
target_compile_options(${PROJECT_NAME}
        PRIVATE -DHAVE_STDINT_H
        PRIVATE -DHAVE_UINT64_T)
if (NOT MSVC)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE -DHAVE_UNISTD_H)
else()
    target_compile_definitions(${PROJECT_NAME}
        PUBLIC -DCAIRO_WIN32_STATIC_BUILD)
endif()
if (EMSCRIPTEN)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE -DCAIRO_NO_MUTEX)
else()
    if (NOT MSVC)
        target_compile_definitions(${PROJECT_NAME}
            PRIVATE -DHAVE_UNISTD_H
            PRIVATE -DCAIRO_HAS_PTHREAD)
     else()
        target_compile_definitions(${PROJECT_NAME}
            PUBLIC -DCAIRO_WIN32_STATIC_BUILD)
    endif()
endif()

message(STATUS "Using local version of ${PROJECT_NAME}")
