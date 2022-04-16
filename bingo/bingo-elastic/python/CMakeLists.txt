find_package(Python3 COMPONENTS Interpreter REQUIRED)

if (NOT Python3_FOUND)
    message(WARNING "Python executable not found, bingo-elastic-python could not be built")
else ()
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} NATIVE_CURRENT_SOURCE_DIR)
    add_custom_target(bingo-elastic-python
            COMMAND ${Python3_EXECUTABLE} -m pip install --force-reinstall --user "${NATIVE_DIST_DIRECTORY}/epam.indigo-*.whl"
            COMMAND ${Python3_EXECUTABLE} -m pylint bingo_elastic
            COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel
            COMMAND ${COPY_COMMAND} ${NATIVE_CURRENT_SOURCE_DIR}${SEP}dist${SEP}*.whl ${NATIVE_DIST_DIRECTORY}${SEP}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
    if (BUILD_INDIGO)
        add_dependencies(bingo-elastic-python indigo-python)
    endif ()
endif ()
