# Locate oci
# This module defines
# OCI_LIBRARY_DIRS
# OCI_INCLUDE_DIRS, where to find the headers

FIND_PATH(OCI_INCLUDE_DIR oci.h
    $ENV{ORACLE_HOME}/rdbms/public
    $ENV{ORACLE_HOME}/oci/include
    $ENV{ORACLE_HOME}/include
    $ENV{ORACLE_HOME}/sdk/include
    $ENV{ORACLE_HOME}/OCI/include
    /usr/include/oracle/*/client64
)
message(STATUS ${OCI_INCLUDE_DIR})

if(WIN32)
  set(OCI_LIBRARY_NAME "oci.lib")
elseif(UNIX)
  set(OCI_LIBRARY_NAME "libclntsh.so")
endif()

FIND_PATH(OCI_LIBRARY_DIR ${OCI_LIBRARY_NAME}
    $ENV{ORACLE_HOME}/lib/
    $ENV{ORACLE_HOME}/oci/lib/msvc/
    /usr/lib/oracle/21/client64/lib
)

message(STATUS ${OCI_LIBRARY_DIR})

set(OCI_FOUND "NO")
if(OCI_LIBRARY_DIR AND OCI_INCLUDE_DIR)
    set(OCI_FOUND "YES")
    set(OCI_LIBRARY ${OCI_LIBRARY_DIR}/${OCI_LIBRARY_NAME})
    message(STATUS "Found OCI at ${OCI_INCLUDE_DIR} and ${OCI_LIBRARY_DIR}")
endif()


