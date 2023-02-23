cmake_minimum_required(VERSION 3.6)

project(indigo-cpp-unit-tests LANGUAGES CXX)

if (ENABLE_TESTS)
    the block is commented out due to presense of memory leaks.
    if(UNIX AND NOT APPLE)
    #    commented out due to memory leaks
    #    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -g")
    #    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize=leak -g")
    #    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=leak")
    #    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fsanitize=address -fsanitize=leak")
    #    set(CMAKE_CXX_COMPILER_LAUNCHER ${CMAKE_COMMAND} -E env ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1)
    endif()
    set(DATA_PATH ${CMAKE_SOURCE_DIR}/data)
    file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/**/*.cpp)

    set(DATA_PATH ${CMAKE_SOURCE_DIR}/data)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/common.h.in ${CMAKE_CURRENT_BINARY_DIR}/common.h)
    message(STATUS ${CMAKE_CURRENT_SOURCE_DIR}/common.h)
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} main.cpp common.cpp)
    target_link_libraries(${PROJECT_NAME} indigo-cpp gtest object_threadsafe)
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
    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} --gtest_output=xml:indigo_cpp_unit_tests.xml)
endif()
