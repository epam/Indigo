cmake_minimum_required(VERSION 3.6)

project(indigo-cpp LANGUAGES CXX)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIUGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME}-object OBJECT ${${PROJECT_NAME}_SOURCES})
# target_compile_definitions(${PROJECT_NAME}-object PRIVATE INDIGO_CPP_DEBUG=1)
target_include_directories(${PROJECT_NAME}-object
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}-object
        PUBLIC indigo indigo-inchi indigo-renderer bingo-nosql)

if (NOT EMSCRIPTEN)
    add_library(${PROJECT_NAME} STATIC $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
    target_include_directories(${PROJECT_NAME}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC indigo indigo-inchi indigo-renderer bingo-nosql)
endif ()
