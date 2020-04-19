set(INDIGO_DEFAULT_VERSION "1.4.0-beta")
set(INDIGO_MAX_REVISION 500)

find_package(Git)
if(GIT_EXECUTABLE)
   EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} describe --long --tags --match indigo-*
                    OUTPUT_VARIABLE INDIGO_FULL_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
 	if (NOT ${INDIGO_FULL_VERSION} STREQUAL "")
		string(REGEX REPLACE "indigo-(.+)-(.+)-(.+)" "\\2" INDIGO_REVISION ${INDIGO_FULL_VERSION})
		if (${INDIGO_REVISION} GREATER ${INDIGO_MAX_REVISION})
			message(SEND_ERROR "Indigo revision ${INDIGO_REVISION} is greater than max revision ${INDIGO_MAX_REVISION}. Please create appropriate version tag" )
		endif()
   		string(REGEX REPLACE "indigo-(.+)-(.+)-(.+)" "\\1.\\2-\\3" INDIGO_FULL_VERSION ${INDIGO_FULL_VERSION})
   		string(REGEX REPLACE "(.+)-(.+)" "\\1" INDIGO_VERSION ${INDIGO_FULL_VERSION})
   	else()
   		set(INDIGO_VERSION "${INDIGO_DEFAULT_VERSION}")
		set(INDIGO_FULL_VERSION "${INDIGO_DEFAULT_VERSION}-00000000")
   	endif()
else()
	set(INDIGO_VERSION "${INDIGO_DEFAULT_VERSION}")
	set(INDIGO_FULL_VERSION "${INDIGO_DEFAULT_VERSION}-00000000")
endif()

message(STATUS "Indigo full version: " ${INDIGO_FULL_VERSION})

# Do not forget to launch build_scripts/indigo-update-version.py after changing the version because it should be ${RV} changed in the Java and .NET files as well

if($ENV{BUILD_NUMBER})
   set(INDIGO_BUILD_VERSION $ENV{BUILD_NUMBER})
else()
   set(INDIGO_BUILD_VERSION 0)
endif()

set(INDIGO_VERSION_EXT "${INDIGO_FULL_VERSION} ${PACKAGE_SUFFIX}")
