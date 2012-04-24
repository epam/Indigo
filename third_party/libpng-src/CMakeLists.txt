cmake_minimum_required(VERSION 2.8)

project(PNG C)

if(USE_SYSTEM_PNG)
    find_package(PNG)
    if (NOT PNG_FOUND)
	     MESSAGE(FATAL_ERROR "Cannot find system png library")
	 endif()
endif()

if (NOT PNG_FOUND)
    message(STATUS "Using local LibPNG library")
    include_directories(include)
    set(PNG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
    aux_source_directory(src PNG_SOURCES)

    if (NOT ZLIB_FOUND)
	     message(STATUS "Finding zlib for libpng")
		  # TODO: if zlib is local that use it too
        find_package(ZLIB)
    endif()
    include_directories(${ZLib_HEADERS_DIR})

    add_library(png STATIC ${PNG_SOURCES})

    set_target_properties(png PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
    target_link_libraries(png z)
    set_property(TARGET png PROPERTY FOLDER "third_party")
	 pack_static(png)
endif()