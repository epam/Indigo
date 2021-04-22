# - Try to find Cairo
# Once done, this will define
#
#  CAIRO_FOUND - system has Cairo
#  CAIRO_INCLUDE_DIRS - the Cairo include directories
#  CAIRO_LIBRARIES - link these to use Cairo
#
# Copyright (C) 2012 Raphael Kubo da Costa <rakuco@webkit.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND ITS CONTRIBUTORS ``AS
# IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ITS
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_CAIRO cairo QUIET)

FIND_PATH(CAIRO_INCLUDE_DIRS
        NAMES cairo.h
        HINTS ${PC_CAIRO_INCLUDEDIR}
        ${PC_CAIRO_INCLUDE_DIRS}
        PATH_SUFFIXES cairo
        )

FIND_LIBRARY(CAIRO_LIBRARIES
        NAMES cairo
        HINTS ${PC_CAIRO_LIBDIR}
        ${PC_CAIRO_LIBRARY_DIRS}
        )

IF (CAIRO_INCLUDE_DIRS)
    IF (EXISTS "${CAIRO_INCLUDE_DIRS}/cairo-version.h")
        FILE(READ "${CAIRO_INCLUDE_DIRS}/cairo-version.h" CAIRO_VERSION_CONTENT)

        STRING(REGEX MATCH "#define +CAIRO_VERSION_MAJOR +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
        SET(CAIRO_VERSION_MAJOR "${CMAKE_MATCH_1}")

        STRING(REGEX MATCH "#define +CAIRO_VERSION_MINOR +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
        SET(CAIRO_VERSION_MINOR "${CMAKE_MATCH_1}")

        STRING(REGEX MATCH "#define +CAIRO_VERSION_MICRO +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
        SET(CAIRO_VERSION_MICRO "${CMAKE_MATCH_1}")

        SET(CAIRO_VERSION "${CAIRO_VERSION_MAJOR}.${CAIRO_VERSION_MINOR}.${CAIRO_VERSION_MICRO}")
    ENDIF ()
ENDIF ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cairo REQUIRED_VARS CAIRO_LIBRARIES CAIRO_INCLUDE_DIRS
                                        VERSION_VAR CAIRO_VERSION)
if(Cairo_FOUND)
    if(NOT TARGET Cairo::Cairo)
        add_library(Cairo::Cairo UNKNOWN IMPORTED)
        set_target_properties(Cairo::Cairo PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${CAIRO_INCLUDE_DIRS}")

        if(Cairo_LIBRARY_RELEASE)
            set_property(TARGET Cairo::Cairo APPEND PROPERTY
                    IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(Cairo::Cairo PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
                    IMPORTED_LOCATION_RELEASE "${Cairo_LIBRARY_RELEASE}")
        endif()

        if(Cairo_LIBRARY_DEBUG)
            set_property(TARGET Cairo::Cairo APPEND PROPERTY
                    IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(Cairo::Cairo PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
                    IMPORTED_LOCATION_DEBUG "${CAIRO_LIBRARIES}")
        endif()

        if(NOT Cairo_LIBRARY_RELEASE AND NOT Cairo_LIBRARY_DEBUG)
            set_target_properties(Cairo::Cairo PROPERTIES
                    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                    IMPORTED_LOCATION "${CAIRO_LIBRARIES}")
        endif()
    endif()
endif()
