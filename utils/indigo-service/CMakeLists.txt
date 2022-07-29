cmake_minimum_required(VERSION 3.6)

project(indigo-service)

if (NOT (UNIX AND NOT APPLE))
    message(WARNING "Indigo-service requires Linux OS to build, since we need indigo-python for Linux")
else()
    find_program(Docker_EXECUTABLE docker)

    if (NOT Docker_EXECUTABLE)
        message(WARNING "Docker executable not found, skipping building indigo-service Docker image")
    else ()
        add_custom_target(${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E copy ${NATIVE_DIST_DIRECTORY}/epam.indigo-*-manylinux1_x86_64.whl ${CMAKE_CURRENT_SOURCE_DIR}/backend/lib/
                COMMAND docker build -f ./Dockerfile . -t epmlsop/indigo-service:latest
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/backend
                DEPENDS indigo-python
                )
    endif()
endif()
