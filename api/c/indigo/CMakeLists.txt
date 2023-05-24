cmake_minimum_required(VERSION 3.6)

project(indigo LANGUAGES C CXX)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/indigo_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/indigo_version.h)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIUGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME}-object OBJECT ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME}-object
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
target_link_libraries(${PROJECT_NAME}-object
        PUBLIC indigo-core)

if (NOT EMSCRIPTEN)
    add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE indigo-core
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER indigo.h)
    install(TARGETS ${PROJECT_NAME})

    if(UNIX AND NOT APPLE)
         target_link_options(${PROJECT_NAME} PRIVATE -Wl,--dynamic-list-cpp-typeinfo -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/libindigo.sym)
    endif()

    if (MINGW)
        # Avoid all MinGW-related runtime-dependencies, including them in DLL
        target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Bstatic,--whole-archive -lpthread -Wl,--no-whole-archive)
    elseif(MSYS OR CYGWIN)
        target_link_options(${PROJECT_NAME} PRIVATE -Wl,--exclude-libs,ALL)
    else()
        # On MSVC we cannot statically link C++ runtime, so we need to copy required files as dependencies
        set(INDIGO_MSVC_SYSTEM_RUNTIME_LIBS "")
        list(APPEND INDIGO_MSVC_SYSTEM_RUNTIME_LIBS_SUBSTRINGS
                vcruntime140_1.dll
                vcruntime140.dll
                msvcp140.dll
        )
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
        include(InstallRequiredSystemLibraries)
        foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
            foreach(indigo_lib_substring ${INDIGO_MSVC_SYSTEM_RUNTIME_LIBS_SUBSTRINGS})
                string(FIND ${lib} ${indigo_lib_substring} INDIGO_LIB_SUBSTRING_FOUND)
                if(lib MATCHES "${indigo_lib_substring}")
                    list(APPEND INDIGO_MSVC_SYSTEM_RUNTIME_LIBS
                            ${lib})
                    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                            COMMAND ${CMAKE_COMMAND} -E copy ${lib} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
                endif()
            endforeach()
        endforeach()
    endif()
    target_include_directories(${PROJECT_NAME}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

    # Prepare libraries to build wrappers
    add_custom_target(before-indigo-wrappers-${PROJECT_NAME}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY})
    foreach(lib ${INDIGO_MSVC_SYSTEM_RUNTIME_LIBS})
        add_custom_command(TARGET before-indigo-wrappers-${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${lib} ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY})
    endforeach()
    add_dependencies(before-indigo-wrappers before-indigo-wrappers-${PROJECT_NAME})
    add_dependencies(before-indigo-wrappers-${PROJECT_NAME} ${PROJECT_NAME})
endif()

if (BUILD_INDIGO_UTILS OR EMSCRIPTEN OR WITH_STATIC)
    add_library(${PROJECT_NAME}-static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
    target_include_directories(${PROJECT_NAME}-static
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(${PROJECT_NAME}-static
            PUBLIC indigo-core)
endif ()
