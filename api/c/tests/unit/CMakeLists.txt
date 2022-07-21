cmake_minimum_required(VERSION 3.6)

project(indigo-api-unit-tests LANGUAGES CXX)

if (ENABLE_TESTS)
    set(DATA_PATH ${CMAKE_SOURCE_DIR}/data)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/common.h.in ${CMAKE_CURRENT_BINARY_DIR}/common.h)

    file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} common.cpp main.cpp)
    target_link_libraries(${PROJECT_NAME} indigo-core indigo bingo-nosql indigo-renderer indigo-inchi gtest)
    target_include_directories(${PROJECT_NAME}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
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
    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} --gtest_output=xml:indigo_unit_tests.xml)
endif()
