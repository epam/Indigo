if(WIN32)
    set(CMAKE_SYSTEM_VERSION 6.1)
endif()

cmake_minimum_required(VERSION 3.9)

project(indigo-all LANGUAGES NONE)

# Options

# Common options
option(ENABLE_TESTS "Enable CTest tests" ON)
option(BUILD_STANDALONE "Build without any system dependencies except for libc, otherwise require tinyxml, zlib, rapidjson, and cairo for renderer" ON)

# Indigo API options
option(BUILD_INDIGO "Build indigo shared library" ON)

# Indigo API Wrappers options
option(BUILD_INDIGO_WRAPPERS "Build all existing Indigo API wrappers" ON)
option(BUILD_INDIGO_WRAPPERS_PYTHON "Build Indigo API wrappers to Python" ON)
option(BUILD_INDIGO_WRAPPERS_JAVA "Build Indigo API wrappers to Java" ON)
option(BUILD_INDIGO_WRAPPERS_DOTNET "Build Indigo API wrappers to .NET" ON)
# option(BUILD_INDIGO_WRAPPERS_R "Build Indigo API wrappers to R [WIP]" OFF)

# CLI utils options
option(BUILD_INDIGO_UTILS "Build indigo-cano, indigo-deco and indigo-depict utilities" ON)

# Bingo options
option(BUILD_BINGO "Build all existing Bingo cartridges" ON)
option(BUILD_BINGO_POSTGRES "Build Bingo Postgres cartridge" ON)
option(BUILD_BINGO_SQLSERVER "Build Bingo MS SqlServer cartridge" ON)
option(BUILD_BINGO_ORACLE "Build Bingo Oracle cartridge" ON)

# Auto-set dependent options

if (BUILD_INDIGO_UTILS AND NOT BUILD_INDIGO)
    message(STATUS "Enabling BUILD_INDIGO because it's required for BUILD_INDIGO_UTILS")
    set(BUILD_INDIGO ON)
endif()

if (BUILD_INDIGO_WRAPPERS)
    message(STATUS "Enabling BUILD_INDIGO_WRAPPERS_PYTHON, BUILD_INDIGO_WRAPPERS_JAVA, BUILD_INDIGO_WRAPPERS_DOTNET because BUILD_INDIGO_WRAPPERS=ON")
    set(BUILD_INDIGO_WRAPPERS_PYTHON ON)
    set(BUILD_INDIGO_WRAPPERS_JAVA ON)
    set(BUILD_INDIGO_WRAPPERS_DOTNET ON)
    # set(BUILD_INDIGO_WRAPPERS_R ON)
else()
    message(STATUS "Disabling BUILD_INDIGO_WRAPPERS_PYTHON, BUILD_INDIGO_WRAPPERS_JAVA, BUILD_INDIGO_WRAPPERS_DOTNET because BUILD_INDIGO_WRAPPERS=OFF")
    set(BUILD_INDIGO_WRAPPERS_PYTHON OFF)
    set(BUILD_INDIGO_WRAPPERS_JAVA OFF)
    set(BUILD_INDIGO_WRAPPERS_DOTNET OFF)
    # set(BUILD_INDIGO_WRAPPERS_R OFF)
endif()

if (BUILD_BINGO)
    message(STATUS "Enabling BUILD_BINGO_POSTGRES, BUILD_BINGO_SQLSERVER, BUILD_BINGO_ORACLE because BUILD_BINGO=ON")
    set(BUILD_BINGO_POSTGRES ON)
    set(BUILD_BINGO_SQLSERVER ON)
    set(BUILD_BINGO_ORACLE ON)
else()
    message(STATUS "Disabling BUILD_BINGO_POSTGRES, BUILD_BINGO_SQLSERVER, BUILD_BINGO_ORACLE because BUILD_BINGO=OFF")
    set(BUILD_BINGO_POSTGRES OFF)
    set(BUILD_BINGO_SQLSERVER OFF)
    set(BUILD_BINGO_ORACLE OFF)
endif()

if (EMSCRIPTEN)
    message(STATUS "Emscripten build: Disabling all except indigo-ketcher and indigo-wasm, and enabling them")
    set(ENABLE_TESTS OFF)
    set(BUILD_STANDALONE ON)
    set(BUILD_INDIGO_WRAPPERS OFF)
    set(BUILD_INDIGO_WRAPPERS_PYTHON OFF)
    set(BUILD_INDIGO_WRAPPERS_JAVA OFF)
    set(BUILD_INDIGO_WRAPPERS_DOTNET OFF)
    # set(BUILD_INDIGO_WRAPPERS_R OFF)
    set(BUILD_INDIGO_UTILS OFF)
    set(BUILD_BINGO_POSTGRES OFF)
    set(BUILD_BINGO_SQLSERVER OFF)
    set(BUILD_BINGO_ORACLE OFF)
endif()

if (BUILD_INDIGO OR BUILD_INDIGO_UTILS OR BUILD_BINGO_SQLSERVER OR BUILD_BINGO_ORACLE OR BUILD_BINGO_POSTGRES OR EMSCRIPTEN)
    set(BUILD_NATIVE ON)
endif()


# Print all options and settings
message(STATUS "ENABLE_TESTS=${ENABLE_TESTS}")
message(STATUS "BUILD_STANDALONE=${BUILD_STANDALONE}")
message(STATUS "BUILD_INDIGO=${BUILD_INDIGO}")
message(STATUS "BUILD_INDIGO_WRAPPERS=${BUILD_INDIGO_WRAPPERS}")
message(STATUS "BUILD_INDIGO_WRAPPERS_PYTHON=${BUILD_INDIGO_WRAPPERS_PYTHON}")
message(STATUS "BUILD_INDIGO_WRAPPERS_JAVA=${BUILD_INDIGO_WRAPPERS_JAVA}")
message(STATUS "BUILD_INDIGO_WRAPPERS_DOTNET=${BUILD_INDIGO_WRAPPERS_DOTNET}")
# message(STATUS "BUILD_INDIGO_WRAPPERS_R=${BUILD_INDIGO_WRAPPERS_R}")
message(STATUS "BUILD_INDIGO_UTILS=${BUILD_INDIGO_UTILS}")
message(STATUS "BUILD_BINGO_POSTGRES=${BUILD_BINGO_POSTGRES}")
message(STATUS "BUILD_BINGO_SQLSERVER=${BUILD_BINGO_SQLSERVER}")
message(STATUS "BUILD_BINGO_ORACLE=${BUILD_BINGO_ORACLE}")

# Set up compiler flags and other preparatory steps
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(setup)

# In subprojects we'll add dependencies for this pseudo-target to build all packages at once
if (BUILD_INDIGO OR BUILD_INDIGO_UTILS OR BUILD_BINGO_POSTGRES OR BUILD_BINGO_ORACLE OR BUILD_BINGO_SQLSERVER)
    add_subdirectory(third_party)
    add_subdirectory(core)
endif()
if (BUILD_INDIGO OR BUILD_INDIGO_WRAPPERS)
    add_subdirectory(api)
endif()
if (BUILD_BINGO_POSTGRES OR BUILD_BINGO_ORACLE OR BUILD_BINGO_SQLSERVER)
    add_subdirectory(bingo)
endif()
if (BUILD_INDIGO_UTILS)
    add_subdirectory(utils)
endif()
