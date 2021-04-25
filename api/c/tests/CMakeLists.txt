cmake_minimum_required(VERSION 3.6)

project(indigo-tests LANGUAGES C CXX)

if (ENABLE_TESTS)
    file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} main.cpp)
    target_link_libraries(${PROJECT_NAME} indigo bingo-nosql indigo-renderer indigo-inchi indigo-core gtest)
    if(MSVC)
        target_link_options(${PROJECT_NAME}
                PRIVATE -force:multiple)
    elseif(APPLE)
        target_link_options(${PROJECT_NAME}
                PRIVATE -Wl,-m)
    elseif(MINGW OR UNIX OR MSYS OR CYGWIN)
        target_link_options(${PROJECT_NAME}
                PRIVATE -Wl,--allow-multiple-definition)
    endif()
    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

    add_executable(${PROJECT_NAME}-dlopen ${CMAKE_CURRENT_SOURCE_DIR}/indigo-test-dlopen.c)
    target_link_libraries(${PROJECT_NAME}-dlopen ${CMAKE_DL_LIBS})
    add_test(NAME ${PROJECT_NAME}-dlopen-indigo
            COMMAND ${PROJECT_NAME}-dlopen ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}indigo${CMAKE_SHARED_LIBRARY_SUFFIX})
    if(BUILD_INDIGO_INCHI)
        add_test(NAME ${PROJECT_NAME}-dlopen-indigo-inchi
                COMMAND ${PROJECT_NAME}-dlopen ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}indigo${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}indigo-inchi${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
    if (BUILD_INDIGO_RENDERER)
        add_test(NAME ${PROJECT_NAME}-dlopen-indigo-renderer
                COMMAND ${PROJECT_NAME}-dlopen ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}indigo${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}indigo-renderer${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
    if (BUILD_BINGO_NOSQL)
        add_test(NAME ${PROJECT_NAME}-dlopen-bingo-nosql
                COMMAND ${PROJECT_NAME}-dlopen ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}indigo${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_SHARED_LIBRARY_PREFIX}bingo-nosql${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
endif()
