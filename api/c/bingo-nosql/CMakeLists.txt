cmake_minimum_required(VERSION 3.4)

project(bingo-nosql LANGUAGES CXX)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIUGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/mmf/*.cpp)

if (NOT EMSCRIPTEN)
    add_library(${PROJECT_NAME}-object OBJECT ${${PROJECT_NAME}_SOURCES})
    target_link_libraries(${PROJECT_NAME}-object
            PRIVATE indigo-object)
    target_include_directories(${PROJECT_NAME}-object
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

    add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC indigo
            PRIVATE indigo-core
    )
    target_include_directories(${PROJECT_NAME}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

    if(MSVC)
        target_link_options(${PROJECT_NAME}
                PRIVATE -force:multiple)
    elseif(APPLE)
        target_link_options(${PROJECT_NAME}
                PRIVATE -Wl,-m)
    elseif(MINGW OR UNIX OR MSYS OR CYGWIN)
        target_link_options(${PROJECT_NAME}
                PRIVATE -Wl,--allow-multiple-definition)
        if(MINGW)
                target_link_options(${PROJECT_NAME}
                        PRIVATE -Wl,-Bstatic,--whole-archive -lpthread -Wl,--no-whole-archive)
        endif()
    endif()

    add_custom_target(before-indigo-wrappers-${PROJECT_NAME}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY})
    add_dependencies(before-indigo-wrappers before-indigo-wrappers-${PROJECT_NAME})
    add_dependencies(before-indigo-wrappers-${PROJECT_NAME} ${PROJECT_NAME})
endif()
