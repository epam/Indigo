cmake_minimum_required(VERSION 3.6)

project(indigo LANGUAGES C CXX)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/indigo_version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/indigo_version.h)

file(GLOB ${PROJECT_NAME}_SOURCES CONFIUGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME}-object OBJECT ${${PROJECT_NAME}_SOURCES})
target_include_directories(${PROJECT_NAME}-object
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}-object
        PUBLIC indigo-core
        PUBLIC Threads::Threads)

add_library(${PROJECT_NAME}-static STATIC $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
target_include_directories(${PROJECT_NAME}-static
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}-static
        PUBLIC indigo-core
        PUBLIC Threads::Threads)

if (NOT EMSCRIPTEN)
    add_library(${PROJECT_NAME} SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-object>)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC indigo-core
            PUBLIC Threads::Threads)
    target_include_directories(${PROJECT_NAME}
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

    if (ENABLE_TESTS)
        add_subdirectory(tests)
    endif ()

    install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION lib)
    install(FILES indigo.h DESTINATION .)

    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION ${INDIGO_VERSION})
    set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME_LOWER}-${CMAKE_SYSTEM_PROCESSOR}")
    if (UNIX)
        set(CPACK_GENERATOR "TGZ")
    else ()
        set(CPACK_GENERATOR "ZIP")
    endif ()
    set(CPACK_VERBATIM_VARIABLES YES)
    set(CPACK_OUTPUT_CONFIG_FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.CPackConfig.cmake)
    include(CPack)

    add_custom_target(package-${PROJECT_NAME}
            COMMAND "${CMAKE_CPACK_COMMAND}"
            "-C" "$<CONFIGURATION>" "--config" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.CPackConfig.cmake")
    add_dependencies(package package-${PROJECT_NAME})

    if (PREPARE_INDIGO_BINDINGS)
        add_custom_target(before-bindings-${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY}
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${INDIGO_CURRENT_NATIVE_LIBS_DIRECTORY})
        add_dependencies(before-bindings before-bindings-${PROJECT_NAME})
        add_dependencies(before-bindings-${PROJECT_NAME} ${PROJECT_NAME})
    endif ()

endif ()
