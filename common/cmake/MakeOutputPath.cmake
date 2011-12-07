
# if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
	# # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
	# # differentiation between debug and release builds.
	# MESSAGE(STATUS "Using default build type as Release")
	# #set(CMAKE_BUILD_TYPE "Release")
	# if(${MSVC})
		# set (CONFIGURATION_NAME "$(ConfigurationName)")
	# else ()
		# MESSAGE(FATAL_ERROR "Not implemented yet. Needs to check default CMAKE_BUILD_TYPE.")
	# endif ()
# else () 
	# set (CONFIGURATION_NAME ${CMAKE_BUILD_TYPE})
# endif ()

# set(COMMON_OUTPUT_PATH dist/${CONFIGURATION_NAME}/${SYSTEM_FOLDER_NAME}/${SUBSYSTEM_FOLDER_NAME})


set(COMMON_OUTPUT_PATH dist/${SYSTEM_FOLDER_NAME}/${SUBSYSTEM_FOLDER_NAME})
