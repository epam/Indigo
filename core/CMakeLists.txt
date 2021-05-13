add_subdirectory(indigo-core)
if (BUILD_INDIGO OR BUILD_INDIGO_UTILS)
    add_subdirectory(render2d)
endif ()
