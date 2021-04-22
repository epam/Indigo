cmake_minimum_required(VERSION 3.6)

project(indigo-ketcher)

add_library(${PROJECT_NAME}-object OBJECT indigo-ketcher.cpp)
target_link_libraries(${PROJECT_NAME}-object indigo-static indigo-inchi-static indigo-renderer-static cppcodec)

set(EMCC_FLAGS
        -Oz -flto --closure 1
        --bind
        -s INITIAL_MEMORY=32mb
        -s ALLOW_MEMORY_GROWTH=1
        -s DISABLE_EXCEPTION_CATCHING=0
        -s SINGLE_FILE=1
        -s MODULARIZE=1
        -s FILESYSTEM=0
        -s ASSERTIONS=1
        -s USE_SDL=0 -s USE_SDL_IMAGE=0 -s USE_SDL_TTF=0 -s USE_SDL_NET=0
        --no-entry)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    list(APPEND EMCC_FLAGS
            -g)
endif ()

add_custom_target(${PROJECT_NAME}-js
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        COMMAND emcc
            -o ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libindigo-ketcher.js
            ${EMCC_FLAGS}
            $<TARGET_OBJECTS:${PROJECT_NAME}-object>
            $<TARGET_FILE:indigo-static> $<TARGET_FILE:indigo-inchi-static> $<TARGET_FILE:indigo-renderer-static> $<TARGET_FILE:render2d> $<TARGET_FILE:cairo> $<TARGET_FILE:png> $<TARGET_FILE:pixman> $<TARGET_FILE:indigo-core> $<TARGET_FILE:inchi> $<TARGET_FILE:tinyxml> $<TARGET_FILE:z>
        DEPENDS ${PROJECT_NAME}-object
        )

add_custom_target(${PROJECT_NAME}-js-test
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.js ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND node test.js
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        DEPENDS ${PROJECT_NAME}-js
        )
