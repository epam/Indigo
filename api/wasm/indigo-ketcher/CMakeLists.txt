cmake_minimum_required(VERSION 3.6)

project(indigo-ketcher)

add_library(${PROJECT_NAME}-object OBJECT indigo-ketcher.cpp)
target_link_libraries(${PROJECT_NAME}-object indigo-static indigo-inchi-static indigo-renderer-static cppcodec)

set(EMCC_FLAGS
#        -Oz -flto --closure 1
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

add_custom_target(${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_C_COMPILER}
        -o ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/indigo-ketcher.js
        ${EMCC_FLAGS}
        $<TARGET_OBJECTS:${PROJECT_NAME}-object>
        $<TARGET_FILE:indigo-static> $<TARGET_FILE:indigo-inchi-static> $<TARGET_FILE:indigo-renderer-static> $<TARGET_FILE:render2d> $<TARGET_FILE:cairo> $<TARGET_FILE:freetype> $<TARGET_FILE:png> $<TARGET_FILE:pixman> $<TARGET_FILE:indigo-core> $<TARGET_FILE:inchi> $<TARGET_FILE:tinyxml2> $<TARGET_FILE:z>
        DEPENDS ${PROJECT_NAME}-object
        )

if (NOT RENDER_ENABLE_CJK)
        add_custom_target(${PROJECT_NAME}-package
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package.json ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND npm install looks-same
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test/ ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND npm test
        COMMAND npm pack
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}
        COMMAND ${COPY_COMMAND} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${SEP}${PROJECT_NAME}*.tgz ${NATIVE_DIST_DIRECTORY}${SEP}
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        DEPENDS ${PROJECT_NAME}
        )
else()
        add_custom_target(${PROJECT_NAME}-package
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package.json ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND npm install looks-same
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test/ ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test-cjk/ ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND npm test
        COMMAND npm run test_cjk
        COMMAND npm pack
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}
        COMMAND ${COPY_COMMAND} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${SEP}${PROJECT_NAME}*.tgz ${NATIVE_DIST_DIRECTORY}${SEP}
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        DEPENDS ${PROJECT_NAME}
        )
endif()
