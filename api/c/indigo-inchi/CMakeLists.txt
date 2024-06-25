cmake_minimum_required(VERSION 3.4)

project(indigo-inchi LANGUAGES CXX)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIUGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME}-object OBJECT ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME}-object PRIVATE indigo-object)
target_include_directories(${PROJECT_NAME}-object
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

if (NOT EMSCRIPTEN)
    add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
    target_link_libraries(${PROJECT_NAME} PRIVATE indigo indigo-core)
    target_include_directories(${PROJECT_NAME}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

    if(MSVC)
        target_link_options(${PROJECT_NAME}
            PRIVATE -force:multiple)
    endif()

    if(UNIX AND NOT APPLE)
        target_link_options(${PROJECT_NAME} PRIVATE -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/src/libindigo-inchi.sym)
    endif()

    if(MINGW)
        target_link_options(${PROJECT_NAME}
                PRIVATE -Wl,--allow-multiple-definition
                PRIVATE -Wl,-Bstatic,--whole-archive -lpthread -Wl,--no-whole-archive)
    endif()

    add_custom_target(before-indigo-wrappers-${PROJECT_NAME}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY})
    add_dependencies(before-indigo-wrappers before-indigo-wrappers-${PROJECT_NAME})
    add_dependencies(before-indigo-wrappers-${PROJECT_NAME} ${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME}-static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
    target_link_libraries(${PROJECT_NAME}-static PUBLIC indigo-static)
    target_include_directories(${PROJECT_NAME}-static
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)
endif()
