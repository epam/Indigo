cmake_minimum_required(VERSION 3.6)

project(indigo LANGUAGES C CXX)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/indigo_version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/indigo_version.h)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIUGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME}-object OBJECT ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME}-object
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}-object
        PUBLIC indigo-core)

add_library(${PROJECT_NAME}-static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
target_include_directories(${PROJECT_NAME}-static
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}-static
        PUBLIC indigo-core)

if (NOT EMSCRIPTEN)
    add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE indigo-core
    )
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

    if (ENABLE_TESTS)
        add_subdirectory(tests)
    endif ()

    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION lib)
    install(FILES indigo.h DESTINATION .)

    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION ${INDIGO_VERSION})
    set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME_LOWER}-${CMAKE_SYSTEM_PROCESSOR_LOWER}")
    if (UNIX)
        set(CPACK_GENERATOR "TGZ")
    else ()
        set(CPACK_GENERATOR "ZIP")
    endif ()
    set(CPACK_VERBATIM_VARIABLES YES)
    set(CPACK_OUTPUT_CONFIG_FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.CPackConfig.cmake)
    include(CPack)

    add_custom_target(package-${PROJECT_NAME}
            COMMAND "${CMAKE_CPACK_COMMAND}"
            "-C" "$<CONFIGURATION>" "--config" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.CPackConfig.cmake")
    add_dependencies(package package-${PROJECT_NAME})

    if (PREPARE_INDIGO_BINDINGS)
        add_custom_target(before-bindings-${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY}
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY})
        foreach(lib ${INDIGO_MSVC_SYSTEM_RUNTIME_LIBS})
            add_custom_command(TARGET before-bindings-${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy ${lib} ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY})
        endforeach()
        add_dependencies(before-bindings before-bindings-${PROJECT_NAME})
        add_dependencies(before-bindings-${PROJECT_NAME} ${PROJECT_NAME})
    endif ()
endif ()
