cmake_minimum_required(VERSION 3.12)

project(bingo-sqlserver LANGUAGES CSharp)

include(CSharpUtilities)

file (GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS Source/*.cs)

add_custom_target(before-${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:bingo-core-c> ${CMAKE_CURRENT_SOURCE_DIR}/Resources/
    DEPENDS bingo-core-c
)

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES} Properties/AssemblyInfo.cs resource.Designer.cs resource.resx)
add_dependencies(${PROJECT_NAME} bingo-core-c before-${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES 
    VS_GLOBAL_AllowUnsafeBlocks true
    VS_GLOBAL_AssemblyOriginatorKeyFile ${CMAKE_CURRENT_SOURCE_DIR}/indigo.snk
    VS_GLOBAL_SignAssembly true
    VS_DOTNET_REFERENCES "System;System.Data;System.Xml"
    DOTNET_TARGET_FRAMEWORK_VERSION v3.5
)


set(BINGO_SQLSERVER_PACKAGE_DIR ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${CMAKE_SYSTEM_NAME_LOWER}-${CMAKE_SYSTEM_PROCESSOR_LOWER}-${BINGO_VERSION})
add_custom_target(package-bingo-sqlserver
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${BINGO_SQLSERVER_PACKAGE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/setup ${BINGO_SQLSERVER_PACKAGE_DIR}/
		COMMAND ${CMAKE_COMMAND} -E make_directory ${BINGO_SQLSERVER_PACKAGE_DIR}/assembly
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${BINGO_SQLSERVER_PACKAGE_DIR}/assembly/
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/sql ${BINGO_SQLSERVER_PACKAGE_DIR}
		COMMAND ${CMAKE_COMMAND} -E tar cvf ${BINGO_SQLSERVER_PACKAGE_DIR}.zip --format=zip ${BINGO_SQLSERVER_PACKAGE_DIR}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}
		COMMAND ${CMAKE_COMMAND} -E copy ${BINGO_SQLSERVER_PACKAGE_DIR}.zip ${DIST_DIRECTORY}/
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		DEPENDS ${PROJECT_NAME}
)
