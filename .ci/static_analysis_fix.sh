#!/bin/sh -eux

# C++

#mkdir build-clang-tidy
#cd build-clang-tidy
#cmake .. -DUSE_CLANG_TIDY=ON
#cd ..
#find . -not -path "./third_party/*" -a \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.c" \) -exec clang-tidy -p build-clang-tidy --fix --fix-errors --header-filter="*third-party*" {} \;
find . -not -path "./third_party/*" -a \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.c" \) -exec clang-format -i {} \;

# Python

export PYTHONPATH=api/python:api/tests/integration:api/http:bingo/bingo-elastic/python:utils/indigo-service/service

black .
isort .
