cmake_minimum_required(VERSION 3.6)

project(indigo-core LANGUAGES C CXX)

if (MSVC)
    string(APPEND CMAKE_C_FLAGS " -WX")
    string(APPEND CMAKE_CXX_FLAGS " -WX")
endif ()

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES CONFIUGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/common/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/*.c

    ${CMAKE_CURRENT_SOURCE_DIR}/graph/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/graph/*.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/molecule/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/molecule/*.cpp

    # while subdirectories other than src have no includes, 
    # this will work, otherwise extra headers may be added 
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/layout/src/*.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/reaction/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/reaction/*.cpp
)

if (MSVC OR MINGW)
    list(FILTER ${PROJECT_NAME}_SOURCES EXCLUDE REGEX .*posix\\.cp*$)
else()
    list(FILTER ${PROJECT_NAME}_SOURCES EXCLUDE REGEX .*win32\\.cp*$)
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
            PUBLIC inchi
            PUBLIC cppcodec)
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
target_link_libraries(${PROJECT_NAME} PUBLIC object_threadsafe)

if (ENABLE_TESTS)
    add_subdirectory(tests)
endif ()
