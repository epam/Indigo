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

add_custom_target(${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        COMMAND emcc
        -o ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/indigo-ketcher.js
        ${EMCC_FLAGS}
        $<TARGET_OBJECTS:${PROJECT_NAME}-object>
        $<TARGET_FILE:indigo-static> $<TARGET_FILE:indigo-inchi-static> $<TARGET_FILE:indigo-renderer-static> $<TARGET_FILE:render2d> $<TARGET_FILE:cairo> $<TARGET_FILE:freetype> $<TARGET_FILE:png> $<TARGET_FILE:pixman> $<TARGET_FILE:indigo-core> $<TARGET_FILE:inchi> $<TARGET_FILE:tinyxml2> $<TARGET_FILE:z>
        DEPENDS ${PROJECT_NAME}-object
        )

add_custom_target(${PROJECT_NAME}-test
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test.js ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test64.cdx ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test64.ket ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/utf8_ref.svg ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/utf8_ref.png ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test_symbols_4_styles_2_sizes.ket ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ketcher_text_panel_regular_ref.png ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ketcher_text_panel_test_regular.ket ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ketcher_text_panel_bold_ref.png ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ketcher_text_panel_test_bold.ket ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ketcher_text_panel_italic_ref.png ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ketcher_text_panel_test_italic.ket ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ketcher_text_panel_bold_italic_ref.png ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/ketcher_text_panel_test_bold_italic.ket ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND npm install looks-same
        COMMAND node test
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        DEPENDS ${PROJECT_NAME}
        )

if(RENDER_ENABLE_CJK)
        add_custom_target(${PROJECT_NAME}-test-cjk
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test_cjk.js ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CJK_characters_ref.png ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CJK_characters_test.ket ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CJK_characters_2_styles_2_sizes_ref.png ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CJK_characters_2_styles_2_sizes_test.ket ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Characters_4_sets_4_styles_ref.png ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Characters_4_sets_4_styles_test.ket ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND npm install looks-same
        COMMAND node test_cjk
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        DEPENDS ${PROJECT_NAME}
        )
endif()


add_custom_target(${PROJECT_NAME}-package
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/package.json ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
        COMMAND npm test
        COMMAND npm pack
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}
        COMMAND ${COPY_COMMAND} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}${SEP}${PROJECT_NAME}*.tgz ${NATIVE_DIST_DIRECTORY}${SEP}
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        DEPENDS ${PROJECT_NAME}-test
        )

if(RENDER_ENABLE_CJK)
        add_dependencies(${PROJECT_NAME}-package ${PROJECT_NAME}-test-cjk)
endif()
