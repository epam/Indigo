cmake_minimum_required(VERSION 3.6)

project(indigo-integration-tests)

add_custom_target(indigo-integration-tests-prepare
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${INDIGO_REPOSITORY_ROOT_PATH}/api/python/indigo/lib
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${INDIGO_NATIVE_LIBS_DIRECTORY} ${INDIGO_REPOSITORY_ROOT_PATH}/api/python/indigo/lib/
#        COMMAND pyinstaller -F test.py -n indigo-integration-tests
#        COMMAND ${CMAKE_COMMAND} -E copy dist/indigo-integration-tests${CMAKE_EXECUTABLE_SUFFIX} .
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS before-bindings
)

#add_executable(indigo-integration-tests IMPORTED)
#set_property(TARGET indigo-integration-tests PROPERTY "${CMAKE_CURRENT_SOURCE_DIR}/indigo-integration-tests${CMAKE_EXECUTABLE_SUFFIX}")
#
#add_dependencies(indigo-integration-tests indigo-integration-tests-prepare)
