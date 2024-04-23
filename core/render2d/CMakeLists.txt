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

if (USE_FONT_MANAGER)
        add_definitions(-DRENDER_USE_FONT_MANAGER)
endif()

if (NOT USE_FONT_MANAGER)
        list(REMOVE_ITEM ${PROJECT_NAME}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/render_font_face_manager.cpp)
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
get_target_property(IndigoCore_INCLUDE_DIRECTORIES indigo-core INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME}
        PRIVATE ${IndigoCore_INCLUDE_DIRECTORIES}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_link_libraries(${PROJECT_NAME} PUBLIC object_threadsafe)
if (USE_FONT_MANAGER)
        target_include_directories(${PROJECT_NAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/third_party/googlefonts/)
endif()

if (RENDER_ENABLE_CJK)
        target_compile_definitions(${PROJECT_NAME} PRIVATE RENDER_ENABLE_CJK)
endif()

if (BUILD_STANDALONE)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC rapidjson
            PUBLIC cairo)
else ()
    find_package(Cairo REQUIRED)
    find_package(RapidJSON REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(Fontconfig REQUIRED)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC RapidJSON::RapidJSON
            PUBLIC ${pkgcfg_lib_PC_CAIRO_pixman-1}
            PUBLIC Freetype::Freetype
            PUBLIC Fontconfig::Fontconfig
            PUBLIC Cairo::Cairo)
endif ()
