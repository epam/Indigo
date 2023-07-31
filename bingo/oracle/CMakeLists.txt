cmake_minimum_required(VERSION 3.4)

if (APPLE)
	message(WARNING "Bingo-Oracle is not supported on macOS")
else()
	project(bingo-oracle LANGUAGES CXX)

	file (GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS 
		${CMAKE_CURRENT_SOURCE_DIR}/src/oracle/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/oracle/*.h
	)

	find_package(OCI REQUIRED)

	add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})
	target_include_directories(${PROJECT_NAME}
			PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
			PUBLIC ${OCI_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} bingo-core)

	if (WIN32)
		target_link_libraries(${PROJECT_NAME} ${OCI_LIBRARIES})
	endif()
	if (MINGW)
		# Avoid all MinGW-related runtime-dependencies, including them in DLL
		target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Bstatic,--whole-archive -lpthread -Wl,--no-whole-archive)
	endif()


	set(BINGO_ORACLE_PACKAGE_DIR ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${CMAKE_SYSTEM_NAME_LOWER}-${CMAKE_SYSTEM_PROCESSOR_LOWER}-${BINGO_VERSION})
	if (WIN32)
		set(BINGO_ORACLE_SETUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/setup/bingo-oracle-install.bat)
		set(ARCHIVE_FORMAT zip)
		set(ARCHIVE_FLAGS cvf)
		set(ARCHIVE_FLAGS_2 --format=zip)
	else()
		set(BINGO_ORACLE_SETUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/setup/bingo-oracle-install.sh)
		set(ARCHIVE_FLAGS cfvz)
		set(ARCHIVE_FORMAT tgz)
	endif()
	add_custom_target(package-bingo-oracle
			COMMAND ${CMAKE_COMMAND} -E remove_directory ${BINGO_ORACLE_PACKAGE_DIR}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${BINGO_ORACLE_PACKAGE_DIR}/lib
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${BINGO_ORACLE_PACKAGE_DIR}/lib
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/sql ${BINGO_ORACLE_PACKAGE_DIR}/sql
			COMMAND ${CMAKE_COMMAND} -E copy ${BINGO_ORACLE_SETUP_SCRIPT} ${BINGO_ORACLE_PACKAGE_DIR}/
			COMMAND ${CMAKE_COMMAND} -E tar ${ARCHIVE_FLAGS} ${BINGO_ORACLE_PACKAGE_DIR}.${ARCHIVE_FORMAT} ${ARCHIVE_FLAGS_2} ${BINGO_ORACLE_PACKAGE_DIR}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}
			COMMAND ${CMAKE_COMMAND} -E copy ${BINGO_ORACLE_PACKAGE_DIR}.${ARCHIVE_FORMAT} ${DIST_DIRECTORY}/
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
			DEPENDS ${PROJECT_NAME}
	)
endif()
