if (BUILD_STANDALONE)
    add_subdirectory(inchi)
    add_subdirectory(rapidjson)
    add_subdirectory(zlib)
    add_subdirectory(tinyxml2)
endif()
if (BUILD_INDIGO OR BUILD_INDIGO_UTILS)
    if (BUILD_STANDALONE)
        add_subdirectory(libpng)
        add_subdirectory(pixman)
        if (EMSCRIPTEN)
            add_subdirectory(freetype)
        endif()
        add_subdirectory(cairo)
    endif()
endif ()
if (EMSCRIPTEN)
    # Required for base64 encoding
    add_subdirectory(cppcodec)
endif()
if (ENABLE_TESTS)
    add_subdirectory(googletest)
endif()

FetchContent_Declare(
        object_threadsafe
        GIT_REPOSITORY https://github.com/AlexeyAB/object_threadsafe.git
        GIT_TAG        940911017993d73ebfe5cb9ebabd3a9dd7bd45b2
)
FetchContent_MakeAvailable(object_threadsafe)
