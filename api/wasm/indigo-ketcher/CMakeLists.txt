cmake_minimum_required(VERSION 3.6)

project(indigo-ketcher)

add_library(${PROJECT_NAME}-object OBJECT indigo-ketcher.cpp)
target_link_libraries(${PROJECT_NAME}-object indigo-static indigo-inchi-static indigo-renderer-static cppcodec)

add_library(${PROJECT_NAME}-object-norender OBJECT indigo-ketcher.cpp)
target_link_libraries(${PROJECT_NAME}-object-norender indigo-static indigo-inchi-static cppcodec)

set_property(TARGET ${PROJECT_NAME}-object-norender
    PROPERTY COMPILE_DEFINITIONS INDIGO_NO_RENDER
)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(EMCC_FLAGS
            -g
            )
else ()
    set(EMCC_FLAGS
            -Oz
            )
endif ()

set(EMCC_COMMON_FLAGS
        --bind
        -s INITIAL_MEMORY=32mb
        -s ALLOW_MEMORY_GROWTH=1
        -s DISABLE_EXCEPTION_CATCHING=0
        -s MODULARIZE=1
        -s FILESYSTEM=1
        -s ASSERTIONS=1
        -s USE_SDL=0 -s USE_SDL_IMAGE=0 -s USE_SDL_TTF=0 -s USE_SDL_NET=0
        --no-entry
        ${EMCC_FLAGS}
        )

set(EMCC_FLAGS_SEPARATE
        ${EMCC_COMMON_FLAGS}
        -s SINGLE_FILE=0
        -s EXPORT_ES6=1
        -s USE_ES6_IMPORT_META=1
        )

set(EMCC_FLAGS
        ${EMCC_COMMON_FLAGS}
        -s SINGLE_FILE=1
        )

set(TARGET_FILES
        $<TARGET_OBJECTS:${PROJECT_NAME}-object>
        $<TARGET_FILE:indigo-static> $<TARGET_FILE:indigo-inchi-static> $<TARGET_FILE:indigo-renderer-static> $<TARGET_FILE:render2d> $<TARGET_FILE:lunasvg> $<TARGET_FILE:plutovg> $<TARGET_FILE:cairo> $<TARGET_FILE:freetype>  $<TARGET_FILE:png> $<TARGET_FILE:pixman> $<TARGET_FILE:indigo-core> $<TARGET_FILE:inchi> $<TARGET_FILE:tinyxml2> $<TARGET_FILE:z>
        )

set(TARGET_FILES_NORENDER
    $<TARGET_OBJECTS:${PROJECT_NAME}-object-norender> $<TARGET_FILE:indigo-static> $<TARGET_FILE:indigo-inchi-static> $<TARGET_FILE:indigo-core> $<TARGET_FILE:inchi> $<TARGET_FILE:tinyxml2> $<TARGET_FILE:png> $<TARGET_FILE:z>
)

add_custom_target(${PROJECT_NAME}-separate
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        COMMAND emcc
        -o ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/indigo-ketcher-separate-wasm.js
        ${EMCC_FLAGS_SEPARATE}
        ${TARGET_FILES}
        DEPENDS ${PROJECT_NAME}-object
        )

add_custom_target(${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        COMMAND emcc
        -o ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/indigo-ketcher.js
        ${EMCC_FLAGS}
        ${TARGET_FILES}
        DEPENDS ${PROJECT_NAME}-object
        )

add_custom_target(${PROJECT_NAME}-separate-norender
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        COMMAND emcc
        -o ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/indigo-ketcher-separate-wasm-norender.js
        ${EMCC_FLAGS_SEPARATE}
        ${TARGET_FILES_NORENDER}
        DEPENDS ${PROJECT_NAME}-object-norender
        )

add_custom_target(${PROJECT_NAME}-norender
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        COMMAND emcc
        -o ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/indigo-ketcher-norender.js
        ${EMCC_FLAGS}
        ${TARGET_FILES_NORENDER}
        DEPENDS ${PROJECT_NAME}-object-norender
        )

if (NOT RENDER_ENABLE_CJK)
    add_custom_target(${PROJECT_NAME}-package
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package.json ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
            COMMAND npm install
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test/ ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
            WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/patch.cmake
            COMMAND npm test
            COMMAND npm pack
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}
            COMMAND ${COPY_COMMAND} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${SEP}${PROJECT_NAME}*.tgz ${NATIVE_DIST_DIRECTORY}${SEP}
            DEPENDS ${PROJECT_NAME} ${PROJECT_NAME}-separate ${PROJECT_NAME}-separate-norender ${PROJECT_NAME}-norender
            )
else ()
    add_custom_target(${PROJECT_NAME}-package
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package.json ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
            COMMAND npm install
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test/ ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test-cjk/ ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
            WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/patch.cmake
            COMMAND npm test
            COMMAND npm run test_cjk
            COMMAND npm pack
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}
            COMMAND ${COPY_COMMAND} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${SEP}${PROJECT_NAME}*.tgz ${NATIVE_DIST_DIRECTORY}${SEP}
            DEPENDS ${PROJECT_NAME} ${PROJECT_NAME}-separate ${PROJECT_NAME}-separate-norender ${PROJECT_NAME}-norender
            )
endif ()
