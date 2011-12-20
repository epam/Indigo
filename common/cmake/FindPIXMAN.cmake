# Locate pixman
# This module defines
# PIXMAN_LIBRARY_DIRS
# PIXMAN_INCLUDE_DIRS, where to find the headers
#

FIND_PATH(PIXMAN_INCLUDE_DIRS pixman.h
    $ENV{PIXMANDIR}/include
    $ENV{PIXMANDIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /usr/include/pixman
    /usr/include/pixman-1
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
)

IF (WIN32)
  SET(LIBPIXMAN "pixman.lib")
ELSEIF(UNIX)
  SET(LIBPIXMAN "libpixman.so")
ELSEIF(APPLE)
  SET(LIBPIXMAN "libpixman.dylib")
ENDIF()

FIND_PATH(PIXMAN_LIBRARY_DIRS ${LIBPIXMAN}
    $ENV{PIXMANDIR}/lib
    $ENV{PIXMANDIR}
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

SET(PIXMAN_FOUND "NO")
IF(PIXMAN_LIBRARY_DIRS AND PIXMAN_INCLUDE_DIRS)
    SET(PIXMAN_FOUND "YES")
ENDIF(PIXMAN_LIBRARY_DIRS AND PIXMAN_INCLUDE_DIRS)
