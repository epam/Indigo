# Locate tinyxml
# This module defines
# TINYXML_LIBRARY_DIRS
# TINYXML_INCLUDE_DIRS, where to find the headers
#


FIND_PATH(TINYXML_INCLUDE_DIRS tinyxml.h
    $ENV{TINYXMLDIR}/include
    $ENV{TINYXMLDIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /usr/include/tinyxml
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
)

IF (WIN32)
  SET(LIBTINYXML "tinyxml.lib")
ELSEIF(UNIX)
  SET(LIBTINYXML "libtinyxml.so")
ELSEIF(APPLE)
  SET(LIBTINYXML "libtinyxml.dylib")
ENDIF()

FIND_PATH(TINYXML_LIBRARY_DIRS ${LIBTINYXML}
    $ENV{TINYXMLDIR}/lib
    $ENV{TINYXMLDIR}
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

SET(TINYXML_FOUND "NO")
IF(TINYXML_LIBRARY_DIRS AND TINYXML_INCLUDE_DIRS)
    SET(TINYXML_FOUND "YES")
ENDIF(TINYXML_LIBRARY_DIRS AND TINYXML_INCLUDE_DIRS)
