cmake_minimum_required(VERSION 3.6)

project(indigo-core-unit-tests LANGUAGES CXX)

if(ENABLE_TESTS)
    set(DATA_PATH ${CMAKE_SOURCE_DIR}/data)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/common.h.in ${CMAKE_CURRENT_BINARY_DIR}/common.h)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_STANDARD 17)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        set(CMAKE_CXX_EXTENSIONS OFF)
        include(FetchContent)
        FetchContent_Declare(
            heapwatcher
            GIT_REPOSITORY "https://github.com/stephanfr/HeapWatcher.git" )

        FetchContent_MakeAvailable(heapwatcher)

        include_directories(
            ${heapwatcher_SOURCE_DIR}/include
            ${heapwatcher_BIN_DIR}
        )
        SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
    endif()
    file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} common.cpp main.cpp)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${PROJECT_NAME} indigo-core gtest HeapWatcher)
    else()
        target_link_libraries(${PROJECT_NAME} indigo-core gtest)
    endif()
        
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
    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} --gtest_output=xml:indigo_core_unit_tests.xml)
endif()
