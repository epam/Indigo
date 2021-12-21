#!/bin/sh -eux

# C++

find . -not -path "./third_party/*" -a \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.c" \) | xargs clang-format --dry-run -Werror -i

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
