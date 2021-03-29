cmake_minimum_required(VERSION 3.4)

project(bingo-oracle LANGUAGES CXX)

file (GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS "src/oracle/*.cpp")

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC /usr/include/oracle/21/client64)
target_link_libraries(${PROJECT_NAME} bingo-core inchi tinyxml zlib)

#if (MSVC)
#	target_link_libraries(${PROJECT odbc32 odbccp32 ${OCI_LIBRARY})
#endif()
