if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()

set(ENV{VCPKG_FORCE_SYSTEM_BINARIES} 1)

# VCPKG
include(FetchContent)
FetchContent_Declare(vcpkg URL "https://github.com/microsoft/vcpkg/archive/refs/tags/2024.03.25.tar.gz")
FetchContent_GetProperties(vcpkg)
if(NOT vcpkg_POPULATED)
    FetchContent_Populate(vcpkg)
    include("${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")
endif()
#
#find_package(GTest)
#find_package(Freetype)
#find_package(ZLIB)
