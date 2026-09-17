cmake_minimum_required(VERSION 3.6)

project(render2d LANGUAGES C CXX)

if (MSVC)
    string(APPEND CMAKE_C_FLAGS " -W4 -WX")
    string(APPEND CMAKE_CXX_FLAGS " -W4 -WX")
endif ()

file(GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)


if (EMSCRIPTEN)
        add_definitions(-DRENDER_EMSCRIPTEN)
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
get_target_property(IndigoCore_INCLUDE_DIRECTORIES indigo-core INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME}
        PRIVATE ${IndigoCore_INCLUDE_DIRECTORIES}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_link_libraries(${PROJECT_NAME} PUBLIC object_threadsafe)
if (RENDER_ENABLE_NOTO_SANS OR RENDER_ENABLE_NOTO_SANS_CJK)
        target_include_directories(${PROJECT_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/third_party/googlefonts/)
endif()

if (RENDER_ENABLE_NOTO_SANS)
        target_compile_definitions(${PROJECT_NAME} PRIVATE RENDER_ENABLE_NOTO_SANS)
endif()

if (RENDER_ENABLE_NOTO_SANS_CJK)
        target_compile_definitions(${PROJECT_NAME} PUBLIC RENDER_ENABLE_NOTO_SANS_CJK)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC lunasvg)

if (BUILD_STANDALONE)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC rapidjson
            PUBLIC cairo
            PUBLIC freetype)
else ()
    find_package(Cairo REQUIRED)
    find_package(RapidJSON REQUIRED)
    find_package(Freetype 2.14.3 REQUIRED)
    target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME}
            PUBLIC RapidJSON::RapidJSON
            PUBLIC Cairo::Cairo
            PUBLIC ${FREETYPE_LIBRARIES})
endif ()
