if (WIN32 OR APPLE)
    # We need PostgreSQL_TYPE_INCLUDE_DIR definition
    cmake_minimum_required(VERSION 3.17)
else()
    cmake_minimum_required(VERSION 3.4)
endif()

find_package(PostgreSQL)

if (NOT PostgreSQL_FOUND)
    if (UNIX AND NOT APPLE)
        message(STATUS "If you need to build Bingo-Postgres, please install Postgres Server via your package manager")
    else()
        if (CMAKE_SYSTEM_NAME_LOWER STREQUAL "darwin")
            set(PG_OS "osx")
        elseif(CMAKE_SYSTEM_NAME_LOWER STREQUAL "windows")
            set(PG_OS "windows")
            if(CMAKE_SYSTEM_PROCESSOR_LOWER STREQUAL "x86_64")
                set(PG_ARCH "-x64")
            else()
                message(WARNING "Bingo-Postgres: unsupported arch: ${CMAKE_SYSTEM_PROCESSOR_LOWER}")
            endif()
        else()
            message(WARNING "Bingo-Postgres: Unsupported os: ${CMAKE_SYSTEM_NAME_LOWER}")
        endif()

        if (PG_OS)
            if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/pgsql)
                set(PG_DIRNAME postgresql-${PostgreSQL_ADDITIONAL_VERSIONS}-1-${PG_OS}${PG_ARCH}-binaries)
                message(STATUS "Bingo-Postgres: downloading ${PG_DIRNAME}...")
                file(DOWNLOAD https://get.enterprisedb.com/postgresql/${PG_DIRNAME}.zip ${CMAKE_CURRENT_BINARY_DIR}/${PG_DIRNAME}.zip)
                execute_process(
                        COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_BINARY_DIR}/${PG_DIRNAME}.zip
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                )
                if (MSVC)
                    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/patches/generic-msvc.h ${CMAKE_CURRENT_BINARY_DIR}/pgsql/server/port/atomics/generic-msvc.h)
                endif()
                file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/pgsql/include/zlib.h)
            endif()
            set(PostgreSQL_ROOT "${CMAKE_CURRENT_BINARY_DIR}/pgsql")
            find_package(PostgreSQL)
        endif()
    endif()
endif()


if (NOT PostgreSQL_FOUND AND UNIX AND NOT APPLE)
    message(WARNING "PostgreSQL not found, Bingo-Postgres could not be built")
else()
    project(bingo-postgres C CXX)

    file(GLOB ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS
            src/pg_am/*.cpp src/pg_am/*.c
            src/pg_common/*.cpp src/pg_common/*.c
            src/pg_core/*.cpp src/pg_core/*.c)
    add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})
    target_link_libraries(${PROJECT_NAME}
            PUBLIC bingo-core-c-static)
    target_include_directories(${PROJECT_NAME}
            PUBLIC ${PostgreSQL_INCLUDE_DIRS}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/pg_common
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/pg_core)

    if (WIN32)
        string(REPLACE "libpq.lib" "postgres.lib" PostgreSQL_LIBRARIES ${PostgreSQL_LIBRARIES})
        if (MSVC)
            target_include_directories(${PROJECT_NAME}
                    PUBLIC ${PostgreSQL_TYPE_INCLUDE_DIR}/port/win32_msvc)
        elseif(MINGW)
            target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Bstatic,--whole-archive -lpthread -Wl,--no-whole-archive)
            target_compile_definitions(${PROJECT_NAME}
                    PRIVATE -DHAVE_LONG_INT_64 -D_TIMEZONE_DEFINED -DHAVE_GETTIMEOFDAY)
        endif()
        target_include_directories(${PROJECT_NAME}
                PUBLIC ${PostgreSQL_TYPE_INCLUDE_DIR}/port/win32)
        target_link_libraries(${PROJECT_NAME}
                PUBLIC ${PostgreSQL_LIBRARIES})
    endif ()

    if (APPLE)
        target_link_options(${PROJECT_NAME} PRIVATE -undefined suppress -flat_namespace)
    endif()

    if (UNIX)
        # We need to explicitly export "Pg_magic_func", we need to remove visibility=hidden for this library
        string(REPLACE "-fvisibility=hidden" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    endif ()

    string(REPLACE "." ";" PostgreSQL_VERSION_LIST ${PostgreSQL_VERSION_STRING})
    list(GET PostgreSQL_VERSION_LIST 0 PostgreSQL_VERSION_MAJOR)
    list(GET PostgreSQL_VERSION_LIST 1 PostgreSQL_VERSION_MINOR)
    if (PostgreSQL_VERSION_MAJOR GREATER 9)
        set(PostgreSQL_VERSION ${PostgreSQL_VERSION_MAJOR})
    else ()
        set(PostgreSQL_VERSION "${PostgreSQL_VERSION_MAJOR}.${PostgreSQL_VERSION_MINOR}")
    endif ()

    set(BINGO_POSTGRES_PACKAGE_DIR ${CMAKE_BINARY_DIR}/${PROJECT_NAME}${PostgreSQL_VERSION_MAJOR}-${CMAKE_SYSTEM_NAME_LOWER}-${CMAKE_SYSTEM_PROCESSOR_LOWER}-${BINGO_VERSION})
    if (WIN32)
        set(BINGO_POSTGRES_SETUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/setup/bingo-pg-install.bat)
        set(ARCHIVE_FORMAT zip)
        set(ARCHIVE_FLAGS cvf)
        set(ARCHIVE_FLAGS_2 --format=zip)
    else()
        set(BINGO_POSTGRES_SETUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/setup/bingo-pg-install.sh)
        set(ARCHIVE_FLAGS cfvz)
        set(ARCHIVE_FORMAT tgz)
    endif()
    add_custom_target(package-bingo-postgres
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${BINGO_POSTGRES_PACKAGE_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${BINGO_POSTGRES_PACKAGE_DIR}/lib
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${BINGO_POSTGRES_PACKAGE_DIR}/lib
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/sql/common ${BINGO_POSTGRES_PACKAGE_DIR}/sql
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/sql/${PostgreSQL_VERSION}/bingo_am.sql.in ${BINGO_POSTGRES_PACKAGE_DIR}/sql/
            COMMAND ${CMAKE_COMMAND} -E copy ${BINGO_POSTGRES_SETUP_SCRIPT} ${BINGO_POSTGRES_PACKAGE_DIR}/
            COMMAND ${CMAKE_COMMAND} -E tar ${ARCHIVE_FLAGS} ${BINGO_POSTGRES_PACKAGE_DIR}.${ARCHIVE_FORMAT} ${ARCHIVE_FLAGS_2} ${BINGO_POSTGRES_PACKAGE_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E copy ${BINGO_POSTGRES_PACKAGE_DIR}.${ARCHIVE_FORMAT} ${DIST_DIRECTORY}/
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${PROJECT_NAME}
            )
endif()
