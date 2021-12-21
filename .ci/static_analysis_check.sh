#!/bin/sh -eux

# C++

#mkdir build-clang-tidy
#cd build-clang-tidy
#cmake .. -DUSE_CLANG_TIDY=ON
#cd ..
#find . -not -path "./third_party/*" -a \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.c" \) -exec clang-tidy -p build-clang-tidy --header-filter="*third-party*" {} \;
clang-format --version
find . -not -path "./third_party/*" -a \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.c" \) -exec clang-format --verbose -Werror --dry-run {} \;

# Python

export PYTHONPATH=api/python:api/tests/integration:api/http:bingo/bingo-elastic/python:utils/indigo-service/service:

black --version
black --verbose --check .
isort --version
isort --verbose --check .
pflake8 --version
pflake8 --verbose .

mypy api/python
mypy api/http
# TODO
# mypy bingo/bingo-elastic/python
# mypy utils/indigo-service/service
