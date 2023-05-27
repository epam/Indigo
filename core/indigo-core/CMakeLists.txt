cmake_minimum_required(VERSION 3.6)

project(indigo-core LANGUAGES C CXX)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIUGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/common/base_c/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/common/base_cpp/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common/gzip/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common/lzw/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common/math/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/graph/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/molecule/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/layout/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/reaction/src/*.cpp)
if (MSVC OR MINGW)
    list(FILTER ${PROJECT_NAME}_SOURCES EXCLUDE REGEX .*posix\.cp*$)
else()
    list(FILTER ${PROJECT_NAME}_SOURCES EXCLUDE REGEX .*win32\.cp*$)
endif()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})

if (NOT EMSCRIPTEN)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
endif()

if (BUILD_STANDALONE)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC z
            PUBLIC tinyxml2
            PUBLIC rapidjson
            PUBLIC inchi)
else()
    find_package(InChI REQUIRED)
    find_package(TinyXML2 REQUIRED)
    find_package(ZLIB REQUIRED)
    find_package(RapidJSON REQUIRED)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC InChI::InChI
            PUBLIC RapidJSON::RapidJSON
            PUBLIC TinyXML2::TinyXML2
            PUBLIC ZLIB::ZLIB)
endif()
target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/common
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
target_link_libraries(${PROJECT_NAME}
        PUBLIC object_threadsafe
        PUBLIC cppcodec)

if (ENABLE_TESTS)
    add_subdirectory(tests)
endif ()
