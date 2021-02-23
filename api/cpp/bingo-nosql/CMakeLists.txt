cmake_minimum_required(VERSION 3.4)

project(bingo-nosql LANGUAGES CXX)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIUGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME}
        PUBLIC indigo)
target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

if (ENABLE_TESTS)
    add_subdirectory(tests)
endif()

if (PREPARE_INDIGO_BINDINGS)
    add_custom_target(before-bindings-${PROJECT_NAME}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY})
    add_dependencies(before-bindings before-bindings-${PROJECT_NAME})
    add_dependencies(before-bindings-${PROJECT_NAME} ${PROJECT_NAME})
endif()
