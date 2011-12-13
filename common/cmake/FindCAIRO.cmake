# Locate cairo
# This module defines
# CAIRO_LIBRARY_DIRS
# CAIRO_INCLUDE_DIRS, where to find the headers
#


FIND_PATH(CAIRO_INCLUDE_DIRS cairo.h
    $ENV{CAIRODIR}/include
    $ENV{CAIRODIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /usr/include/cairo
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
)

IF (WIN32)
  SET(LIBCAIRO "cairo.lib")
ELSEIF(UNIX)
  SET(LIBCAIRO "libcairo.so")
ELSEIF(APPLE)
  SET(LIBCAIRO "libcairo.dylib")
ENDIF()

FIND_PATH(CAIRO_LIBRARY_DIRS ${LIBCAIRO}
    $ENV{CAIRODIR}/lib
    $ENV{CAIRODIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
)

SET(CAIRO_FOUND "NO")
IF(CAIRO_LIBRARY_DIRS AND CAIRO_INCLUDE_DIRS)
    SET(CAIRO_FOUND "YES")
ENDIF(CAIRO_LIBRARY_DIRS AND CAIRO_INCLUDE_DIRS)
