#!/bin/sh -eux

cmake \
    -D CMAKE_BUILD_TYPE=Release \
    -D CMAKE_SYSTEM_PREFIX_PATH=${PREFIX} \
    -D CMAKE_INSTALL_PREFIX=${PREFIX} \
    -D BUILD_STANDALONE=OFF \
    -D BUILD_INDIG=OFF \
    -D BUILD_INDIGO_PYTHON=ON \
    -D BUILD_INDIGO_WRAPPERS_JAVA=OFF \
    -D BUILD_INDIGO_WRAPPERS_DOTNET=OFF \
    -D BUILD_INDIGO_UTILS=OFF \
    -D BUILD_BINGO=OFF \
    -D BUILD_BINGO_ELASTIC=OFF \
    .

cmake --build . --target indigo-python
pip install ./dist/indigo-python*.whl
