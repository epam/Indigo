cmake_minimum_required(VERSION 3.4)

project(bingo-nosql LANGUAGES CXX)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIUGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

if (NOT EMSCRIPTEN)
    add_library(${PROJECT_NAME}-object OBJECT ${${PROJECT_NAME}_SOURCES})
    target_link_libraries(${PROJECT_NAME}-object
            PRIVATE indigo-object)
    target_include_directories(${PROJECT_NAME}-object
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

    add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC indigo
    )
    if(MSVC)
        target_link_libraries(${PROJECT_NAME}
                PRIVATE indigo-core
        )
        target_link_options(${PROJECT_NAME}
                PRIVATE -force:multiple)
    endif()

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
endif()