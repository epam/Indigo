cmake_minimum_required(VERSION 3.6)

project(render2d LANGUAGES C CXX)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})
get_target_property(IndigoCore_INCLUDE_DIRECTORIES indigo-core INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME} PUBLIC ${IndigoCore_INCLUDE_DIRECTORIES})

if (BUILD_SELF_SUFFICIENT)
    if (NOT EMSCRIPTEN)
        if (CONAN_EXPORTED) # in conan local cache
            include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
            conan_basic_setup(TARGETS)
        else ()
            set(CONAN_CAIRO_OPTIONS
                    cairo:shared=False
                    )
            if (APPLE)
                list(APPEND CONAN_CAIRO_OPTIONS
                        cairo:with_freetype=False
                        cairo:with_fontconfig=False
                        )
            elseif (UNIX)
                list(APPEND CONAN_CAIRO_OPTIONS
                        cairo:with_xlib=False
                        cairo:with_xcb=False
                        cairo:with_xlib_xrender=False
                        cairo:with_glib=False
                        )
#            elseif (WIN32)
#                list(APPEND CONAN_CAIRO_OPTIONS
#                        cairo:with_freetype=False
#                        cairo:with_glib=False
#                        )
            endif ()

            conan_cmake_run(REQUIRES
                    cairo/1.17.4
                    BASIC_SETUP CMAKE_TARGETS
                    BUILD missing
                    OPTIONS ${CONAN_CAIRO_OPTIONS})
        endif ()
        target_link_libraries(${PROJECT_NAME}
                PUBLIC CONAN_PKG::cairo)
    else()
        target_link_libraries(${PROJECT_NAME}
                PUBLIC cairo)
    endif ()
else ()
    find_package(Cairo REQUIRED)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC Cairo::Cairo)
endif ()

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
