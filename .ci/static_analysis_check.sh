#!/bin/sh -eux

# C++

#mkdir build-clang-tidy
#cd build-clang-tidy
#cmake .. -DUSE_CLANG_TIDY=ON
#cd ..
#find . -not -path "./third_party/*" -a \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.c" \) -exec clang-tidy -p build-clang-tidy --header-filter="*third-party*" {} \;
find . -not -path "./third_party/*" -a \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.c" \) -exec clang-format -Werror --dry-run {} \;

# Python

export PYTHONPATH=api/python:api/tests/integration:api/http:bingo/bingo-elastic/python:utils/indigo-service/service:

black --check .
isort --check .
pflake8 .

mypy api/python
mypy api/http
# TODO
# mypy bingo/bingo-elastic/python
# mypy utils/indigo-service/service
