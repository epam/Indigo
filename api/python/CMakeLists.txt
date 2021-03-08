find_package (Python3 COMPONENTS Interpreter REQUIRED)

add_custom_target(bindings-python
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_SOURCE_DIR}/dist ${CMAKE_CURRENT_SOURCE_DIR}/indigo/lib
        COMMAND ${Python3_EXECUTABLE} setup.py test
        COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=macosx_10_7_intel --python-tag=py2.py3
        COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=manylinux1_x86_64 --python-tag=py2.py3
        COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=manylinux1_i686 --python-tag=py2.py3
        COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=win_amd64 --python-tag=py2.py3
        COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel --plat-name=win32 --python-tag=py2.py3
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/dist/*.whl ${DIST_DIRECTORY}/
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
add_dependencies(bindings-python before-bindings)
add_dependencies(bindings bindings-python)
