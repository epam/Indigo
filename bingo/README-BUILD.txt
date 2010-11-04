1. Copy the OCI files

  On all systems:
    Copy oci.h, ociextp.h, and other header files from your Oracle installation
    (usually located in $ORACLE_HOME/rdbms/public/ and/or
    $ORACLE_HOME/oci/include/ and/or $ORACLE_HOME/rdbms/demo/) into
    ../common/oci/include directory. On Solaris, copy them to
    ../common/oci/include.sun directory

  On 32-bit Windows:
    Copy oci.lib from your Oracle installation
    (usually located in %ORACLE_HOME%\oci\lib\msvc\) into
    ../common/oci/lib32/oci32.lib
    (note that you should change the file name)

  On 64-bit Windows:
    Copy oci.lib from your Oracle installation
    (usually located in %ORACLE_HOME%\oci\lib\msvc\) into
    ../common/oci/lib64/oci64.lib
    (note that you should change the file name)

  On Mac OS X:
    Copy liborasdk.dylib from your Oracle installation
    (usually located in %ORACLE_HOME%\lib) into
    ../common/oci/lib64/liborasdk.dylib

2. Build

  Note: you should have the "zip" command-line utility installed on your system.

  On Linux, run

    ./all-release-linux.sh $version

  On Windows, run

    all-release-windows.bat $version

  On Mac OS X, run

    ./all-release-osx.sh $version

  On Solaris, run:

    ./all-release-sun.sh $version

  You will get a number of .zip archives for your selected platform,
  both 32-bit and 64-bit versions.
  (In case of Mac OS X, both 10.5 and 10.6 versions.)

