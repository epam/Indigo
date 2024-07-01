cmake_minimum_required(VERSION 3.6)

project(indigo-integration-tests)

find_package(Python3 COMPONENTS Interpreter Development)

if (NOT Python3_FOUND)
    message(WARNING "Python3 development files or interpreter not found, skip building indigo-integration-tests")
else()
    if (MINGW)
        set(Python_ROOT_DIR "C:/msys64/mingw64")
        set(Python_FIND_REGISTRY NEVER)
        set(Python_FIND_STRATEGY LOCATION)
    endif()
    execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "import os; import sys; print(os.pathsep.join(sys.path).replace(os.sep,'/') + os.pathsep, end='')"
            OUTPUT_VARIABLE PYTHON_SYS_PATH)
    set(PYTHON_SYS_PATH "${CMAKE_CURRENT_SOURCE_DIR}${PYTHON_SYS_PATH}")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/_c/test.h.in ${CMAKE_CURRENT_BINARY_DIR}/_c/test.h)

    add_custom_target(${PROJECT_NAME}-prepare
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${INDIGO_REPOSITORY_ROOT_PATH}/api/python/indigo/lib
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${INDIGO_NATIVE_LIBS_DIRECTORY} ${INDIGO_REPOSITORY_ROOT_PATH}/api/python/indigo/lib/
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS before-indigo-wrappers
    )

    add_executable(${PROJECT_NAME} _c/test.c ${CMAKE_CURRENT_BINARY_DIR}/_c/test.h)
    target_link_libraries(${PROJECT_NAME} Python3::Python)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/_c)
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-prepare)
endif()
