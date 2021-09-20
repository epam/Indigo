cmake_minimum_required(VERSION 3.4)

if (BUILD_INDIGO)
    include(indigo-version.cmake)

    add_custom_target(before-indigo-wrappers ALL)
    set(INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY ${INDIGO_NATIVE_LIBS_DIRECTORY}/${CMAKE_SYSTEM_NAME_LOWER}-${CMAKE_SYSTEM_PROCESSOR_LOWER})

    add_subdirectory(c/indigo)
    add_subdirectory(c/indigo-inchi)
    add_subdirectory(c/indigo-renderer)
    add_subdirectory(c/bingo-nosql)

    add_subdirectory(cpp)

    if (ENABLE_TESTS)
        add_subdirectory(c/tests/dlopen)
        add_subdirectory(c/tests/unit)
        add_subdirectory(cpp/tests)
        add_subdirectory(tests/integration)
    endif ()

    if (EMSCRIPTEN)
        add_subdirectory(wasm/indigo-ketcher)
    endif ()

endif()

if (BUILD_INDIGO_WRAPPERS)
    add_custom_target(indigo-wrappers ALL)
    if (BUILD_INDIGO_WRAPPERS_PYTHON)
        add_subdirectory(python)
    endif ()
    if (BUILD_INDIGO_WRAPPERS_JAVA)
        add_subdirectory(java)
    endif ()
    if (BUILD_INDIGO_WRAPPERS_DOTNET)
        add_subdirectory(dotnet)
    endif ()
    if (BUILD_INDIGO_WRAPPERS_R)
        add_subdirectory(r)
    endif ()
endif ()
