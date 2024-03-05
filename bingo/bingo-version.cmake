set(BINGO_DEFAULT_VERSION "1.9.1")
set(BINGO_MAX_REVISION 1000)

find_package(Git)
if(GIT_EXECUTABLE)
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} describe --long --tags --match indigo-*
            OUTPUT_VARIABLE BINGO_FULL_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    if (NOT ${BINGO_FULL_VERSION} STREQUAL "")
        string(REGEX REPLACE "indigo-(.+)-(.+)-(.+)" "\\2" BINGO_REVISION ${BINGO_FULL_VERSION})
        if (${BINGO_REVISION} GREATER ${BINGO_MAX_REVISION})
            message(SEND_ERROR "Bingo revision ${BINGO_REVISION} is greater than max revision ${BINGO_MAX_REVISION}. Please create appropriate version tag" )
        endif()
        string(REGEX REPLACE "indigo-(.+)-(.+)-(.+)" "\\1.\\2-\\3" BINGO_FULL_VERSION ${BINGO_FULL_VERSION})
        string(REGEX REPLACE "(.+)-(.+)" "\\1" BINGO_VERSION ${BINGO_FULL_VERSION})
    else()
        set(BINGO_VERSION "${BINGO_DEFAULT_VERSION}")
        set(BINGO_FULL_VERSION "${BINGO_DEFAULT_VERSION}-00000000")
    endif()
else()
    set(BINGO_VERSION "${BINGO_DEFAULT_VERSION}")
    set(BINGO_FULL_VERSION "${BINGO_DEFAULT_VERSION}-00000000")
endif()

message(STATUS "Bingo full version: " ${BINGO_FULL_VERSION})

# Do not forget to launch build_scripts/bingo-update-version.py after changing the version because it should be ${RV} changed in the Java and .NET files as well

if($ENV{BUILD_NUMBER})
    set(BINGO_BUILD_VERSION $ENV{BUILD_NUMBER})
else()
    set(BINGO_BUILD_VERSION 0)
endif()

set(BINGO_VERSION_EXT "${BINGO_FULL_VERSION}")
