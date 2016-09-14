find_package(Git)
if(GIT_EXECUTABLE) 
   	EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} describe --long --tags --match "indigo-*" 
                    OUTPUT_VARIABLE INDIGO_FULL_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
 	if (${INDIGO_FULL_VERSION})
   		string(REGEX REPLACE "indigo-(.+)-(.+)-(.+)" "\\1.r\\2-\\3" INDIGO_FULL_VERSION ${INDIGO_FULL_VERSION})
   		string(REGEX REPLACE "(.+)-(.+)" "\\1" INDIGO_VERSION ${INDIGO_FULL_VERSION})
   	else()
   		set(INDIGO_VERSION "1.2.4dev.r0")
		  set(INDIGO_FULL_VERSION "1.2.4dev.r0")
   	endif()
else()
	set(INDIGO_VERSION "1.2.4dev.r0")
	set(INDIGO_FULL_VERSION "1.2.4dev.r0")
endif()

message(STATUS "Indigo full version: " ${INDIGO_FULL_VERSION})

# Do not forget to launch build_scripts/indigo-update-version.py after changing the version because it should be ${RV} changed in the Java and .NET files as well

if($ENV{BUILD_NUMBER})
   set(INDIGO_BUILD_VERSION $ENV{BUILD_NUMBER})
else()
   set(INDIGO_BUILD_VERSION 0)
endif()

set(INDIGO_VERSION_EXT "${INDIGO_FULL_VERSION} ${PACKAGE_SUFFIX}")
