cmake_minimum_required(VERSION 3.4)

project(bingo-oracle LANGUAGES CXX)

if (APPLE)
	message(SEND_ERROR "bingo-oracle is not supported on macOS")
endif()

file (GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS "src/oracle/*.cpp")

find_package(OCI REQUIRED)

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC ${OCI_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} bingo-core inchi tinyxml zlib)

if (WIN32)
	target_link_libraries(${PROJECT_NAME} odbc32 odbccp32 ${OCI_LIBRARIES})
endif()
