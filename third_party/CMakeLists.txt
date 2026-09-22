add_subdirectory(libpng)

option(LUNASVG_BUILD_EXAMPLES OFF)
add_subdirectory(lunasvg)

if (BUILD_STANDALONE)
    # InChI don't have a Conan package yet
    add_subdirectory(cppcodec)
    add_subdirectory(inchi)
    add_subdirectory(rapidjson)
    add_subdirectory(zlib)
    add_subdirectory(tinyxml2)
endif()
if (BUILD_INDIGO OR BUILD_INDIGO_UTILS)
    if (BUILD_STANDALONE)
        add_subdirectory(pixman)
        # Force FreeType onto its bundled zlib: find_package(ZLIB) could otherwise pick up a
        # system zlib mismatched with the vendored one libpng links, corrupting the heap.
        option(FT_DISABLE_ZLIB "Disable use of system zlib and use internal zlib library instead." ON)
        add_subdirectory(freetype)
        add_subdirectory(cairo)
    endif()
endif ()
if (ENABLE_TESTS)
    add_subdirectory(googletest)
endif()

add_subdirectory(object_threadsafe)
