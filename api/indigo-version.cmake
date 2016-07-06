# Do not forget to launch build_scripts/indigo-update-version.py after changing the version because it should be ${RV} changed in the Java and .NET files as well

IF($ENV{BUILD_NUMBER})
   SET(INDIGO_BUILD_VERSION $ENV{BUILD_NUMBER})
ELSE()
   SET(INDIGO_BUILD_VERSION 0)
ENDIF()

SET(INDIGO_VERSION_EXT "${INDIGO_FULL_VERSION} ${PACKAGE_SUFFIX}")
