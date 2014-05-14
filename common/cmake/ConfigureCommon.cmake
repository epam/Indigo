if (COMMON_CONFIGURED)
    return()
endif()

include(GetSystemVersion)
include(SetBuildParameters)
include(MakeOutputPath)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(COMMON_CONFIGURED TRUE)
