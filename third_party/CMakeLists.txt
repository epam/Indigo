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
        # Preserve Indigo's FreeType feature set when using a clean CMake cache.
        option(FT_DISABLE_BZIP2 "Disable FreeType BZip2 support" ON)
        option(FT_DISABLE_PNG "Disable FreeType PNG support" ON)
        option(FT_DISABLE_HARFBUZZ "Disable FreeType HarfBuzz support" ON)
        option(FT_DISABLE_BROTLI "Disable FreeType Brotli support" ON)
        # FreeType's install export cannot include Indigo's local zlib target.
        set(SKIP_INSTALL_ALL ON)
        add_subdirectory(freetype)
        unset(SKIP_INSTALL_ALL)
        add_subdirectory(cairo)
    endif()
endif ()
if (ENABLE_TESTS)
    add_subdirectory(googletest)
endif()

add_subdirectory(object_threadsafe)
