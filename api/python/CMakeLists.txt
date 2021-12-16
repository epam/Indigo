find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(NOT Python3_FOUND)
        message(WARNING "Python executable not found, indigo-python could not be built")
else()
    file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR} NATIVE_CURRENT_SOURCE_DIR)
    add_custom_target(indigo-python
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/dist
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/indigo/lib
            COMMAND ${Python3_EXECUTABLE} setup.py test
            COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=macosx_11_0_arm64 --python-tag=py2.py3
            COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=macosx_10_7_intel --python-tag=py2.py3
            COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=manylinux2014_aarch64 --python-tag=py2.py3
            COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=manylinux1_x86_64 --python-tag=py2.py3
            COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=manylinux1_i686 --python-tag=py2.py3
            COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=win_amd64 --python-tag=py2.py3
            COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=win32 --python-tag=py2.py3
            COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=mingw --python-tag=py2.py3
            COMMAND ${COPY_COMMAND} ${NATIVE_CURRENT_SOURCE_DIR}${SEP}dist${SEP}*.whl ${NATIVE_DIST_DIRECTORY}${SEP}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
    add_dependencies(indigo-wrappers indigo-python)
    if (BUILD_INDIGO)
        add_dependencies(indigo-python before-indigo-wrappers)
    endif ()
endif()
