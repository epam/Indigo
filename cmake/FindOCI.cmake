FIND_PATH(OCI_INCLUDE_DIRS oci.h
        $ENV{ORACLE_HOME}/rdbms/public
        $ENV{ORACLE_HOME}/oci/include
        $ENV{ORACLE_HOME}/include
        $ENV{ORACLE_HOME}/sdk/include
        $ENV{ORACLE_HOME}/OCI/include
        /usr/include/oracle/*/client64
        $ENV{ORACLE_HOME}/sdk/include
        ${CMAKE_BINARY_DIR}/instantclient_${INSTANT_CLIENT_VERSION_MAJOR}_${INSTANT_CLIENT_VERSION_MINOR}/sdk/include
        )

if (NOT OCI_INCLUDE_DIRS)
    # Download OCI manually
    if (NOT OCI_FOUND)
        if (WIN32)
            set(INSTANT_CLIENT_VERSION_MAJOR 19)
            set(INSTANT_CLIENT_VERSION_MINOR 9)
            set(INSTANT_CLIENT_VERSION_PATCH 00)
            set(INSTANT_CLIENT_OS windows)
            set(INSTANT_CLIENT_OS_2 nt)
            set(INSTANT_CLIENT_FILENAME_SUFFIX dbru)
        else ()
            set(INSTANT_CLIENT_VERSION_MAJOR 21)
            set(INSTANT_CLIENT_VERSION_MINOR 1)
            set(INSTANT_CLIENT_VERSION_PATCH 000)
            set(INSTANT_CLIENT_OS linux)
            set(INSTANT_CLIENT_OS_2 linux)
            set(INSTANT_CLIENT_FILENAME_SUFFIX "")
        endif ()
        set(INSTANT_CLIENT_SDK_URL "https://download.oracle.com/otn_software/${INSTANT_CLIENT_OS_2}/instantclient/${INSTANT_CLIENT_VERSION_MAJOR}${INSTANT_CLIENT_VERSION_MINOR}${INSTANT_CLIENT_VERSION_PATCH}/instantclient-sdk-${INSTANT_CLIENT_OS}.x64-${INSTANT_CLIENT_VERSION_MAJOR}.${INSTANT_CLIENT_VERSION_MINOR}.0.0.0${INSTANT_CLIENT_FILENAME_SUFFIX}.zip")
        set(INSTANT_CLIENT_SDK_LOCAL_PATH "${CMAKE_BINARY_DIR}/instantclient-sdk_${INSTANT_CLIENT_VERSION_MAJOR}_${INSTANT_CLIENT_VERSION_MINOR}.zip")
        message(STATUS "Downloading Instant Client SDK from ${INSTANT_CLIENT_SDK_URL}")
        file(DOWNLOAD ${INSTANT_CLIENT_SDK_URL} ${INSTANT_CLIENT_SDK_LOCAL_PATH})
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E tar xzf ${INSTANT_CLIENT_SDK_LOCAL_PATH}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        if (MINGW)
            # MinGW GCC cannot link to MSVC lib file, but can link directly to oci.dll
            set(INSTANT_CLIENT_BASIC_URL "https://download.oracle.com/otn_software/${INSTANT_CLIENT_OS_2}/instantclient/${INSTANT_CLIENT_VERSION_MAJOR}${INSTANT_CLIENT_VERSION_MINOR}${INSTANT_CLIENT_VERSION_PATCH}/instantclient-basic-${INSTANT_CLIENT_OS}.x64-${INSTANT_CLIENT_VERSION_MAJOR}.${INSTANT_CLIENT_VERSION_MINOR}.0.0.0${INSTANT_CLIENT_FILENAME_SUFFIX}.zip")
            set(INSTANT_CLIENT_BASIC_LOCAL_PATH "${CMAKE_BINARY_DIR}/instantclient-basic_${INSTANT_CLIENT_VERSION_MAJOR}_${INSTANT_CLIENT_VERSION_MINOR}.zip")
            message(STATUS "Downloading Instant Client Basic from ${INSTANT_CLIENT_BASIC_URL}")
            file(DOWNLOAD ${INSTANT_CLIENT_BASIC_URL} ${INSTANT_CLIENT_BASIC_LOCAL_PATH})
            execute_process(
                    COMMAND ${CMAKE_COMMAND} -E tar xzf ${INSTANT_CLIENT_BASIC_LOCAL_PATH}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
        endif ()
    endif ()
endif()

FIND_PATH(OCI_INCLUDE_DIRS oci.h
        $ENV{ORACLE_HOME}/rdbms/public
        $ENV{ORACLE_HOME}/oci/include
        $ENV{ORACLE_HOME}/include
        $ENV{ORACLE_HOME}/sdk/include
        $ENV{ORACLE_HOME}/OCI/include
        /usr/include/oracle/*/client64
        $ENV{ORACLE_HOME}/sdk/include
        ${CMAKE_BINARY_DIR}/instantclient_${INSTANT_CLIENT_VERSION_MAJOR}_${INSTANT_CLIENT_VERSION_MINOR}/sdk/include
        )

if (MSVC)
    set(OCI_LIBRARY_NAME oci.lib)
elseif (MINGW)
    set(OCI_LIBRARY_NAME oci.dll)
endif ()

if (OCI_LIBRARY_NAME)
    find_path(OCI_LIBRARY_DIRS ${OCI_LIBRARY_NAME}
            $ENV{ORACLE_HOME}/lib
            $ENV{ORACLE_HOME}/oci/lib/msvc
            /usr/lib/oracle/*/client64/lib
            $ENV{ORACLE_HOME}/sdk/lib
            $ENV{ORACLE_HOME}/sdk/lib/msvc
            ${CMAKE_BINARY_DIR}/instantclient_${INSTANT_CLIENT_VERSION_MAJOR}_${INSTANT_CLIENT_VERSION_MINOR}
            ${CMAKE_BINARY_DIR}/instantclient_${INSTANT_CLIENT_VERSION_MAJOR}_${INSTANT_CLIENT_VERSION_MINOR}/sdk/lib/msvc
            )
    if (OCI_LIBRARY_DIRS)
        set(OCI_LIBRARIES ${OCI_LIBRARY_DIRS}/${OCI_LIBRARY_NAME})
    endif ()
endif()

include(FindPackageHandleStandardArgs)
if (OCI_LIBRARY_NAME)
    find_package_handle_standard_args(OCI REQUIRED_VARS OCI_INCLUDE_DIRS OCI_LIBRARIES)
else()
    find_package_handle_standard_args(OCI REQUIRED_VARS OCI_INCLUDE_DIRS)
endif()
