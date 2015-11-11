SET(INDIGO_VERSION "1.2.2beta-r37")
# Do not forget to launch build_scripts/indigo-update-version.py after changing the version because it should be changed in the Java and .NET files as well

IF($ENV{BUILD_NUMBER})
   SET(INDIGO_BUILD_VERSION $ENV{BUILD_NUMBER})
ELSE()
   SET(INDIGO_BUILD_VERSION 0)
ENDIF()

SET(INDIGO_VERSION_EXT "${INDIGO_VERSION}.${INDIGO_BUILD_VERSION} ${PACKAGE_SUFFIX}")
