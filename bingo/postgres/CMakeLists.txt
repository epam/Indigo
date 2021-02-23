cmake_minimum_required(VERSION 3.4)

project(bingo-postgres C CXX)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS
        src/pg_am/*.cpp src/pg_am/*.c
        src/pg_common/*.cpp src/pg_common/*.c
        src/pg_core/*.cpp src/pg_core/*.c)
find_package(PostgreSQL REQUIRED)
add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME}
        PUBLIC bingo-core-c-static
        PUBLIC PostgreSQL::PostgreSQL
        PUBLIC Threads::Threads)
target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/pg_common
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/pg_core)
if (UNIX)
    # We need to explicitly export "Pg_magic_func", we need to remove visibility=hidden for this library
    string(REPLACE "-fvisibility=hidden" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
endif()

string(REPLACE "." ";" PostgreSQL_VERSION_LIST ${PostgreSQL_VERSION_STRING})
list(GET PostgreSQL_VERSION_LIST 0 PostgreSQL_VERSION_MAJOR)
list(GET PostgreSQL_VERSION_LIST 1 PostgreSQL_VERSION_MINOR)
if(PostgreSQL_VERSION_MAJOR GREATER 9)
    set(PostgreSQL_VERSION ${PostgreSQL_VERSION_MAJOR})
else()
    set(PostgreSQL_VERSION "${PostgreSQL_VERSION_MAJOR}.${PostgreSQL_VERSION_MINOR}")
endif()

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        COMPONENT ${PROJECT_NAME})
install(DIRECTORY sql/common/ DESTINATION sql
        COMPONENT ${PROJECT_NAME})
install(DIRECTORY sql/${PostgreSQL_VERSION}/ DESTINATION sql
        COMPONENT ${PROJECT_NAME})
install(FILES bingo-pg-install.bat bingo-pg-install.sh README.md DESTINATION .
        COMPONENT ${PROJECT_NAME})
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}${PostgreSQL_VERSION}")
set(CPACK_PACKAGE_VERSION ${BINGO_VERSION})
string(TOLOWER ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_NAME_LOWER)
set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME_LOWER}-${CMAKE_SYSTEM_PROCESSOR}")

if (UNIX)
    set(CPACK_GENERATOR "TGZ")
else()
    set(CPACK_GENERATOR "ZIP")
endif()
set(CPACK_VERBATIM_VARIABLES YES)
message(STATUS ${CPACK_OUTPUT_CONFIG_FILE})
set(CPACK_OUTPUT_CONFIG_FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.CPackConfig.cmake)

include(CPack)

add_custom_target(package-${PROJECT_NAME}
        COMMAND "${CMAKE_CPACK_COMMAND}"
        "-C" "$<CONFIGURATION>" "--config" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.CPackConfig.cmake")
add_dependencies(package package-${PROJECT_NAME})
