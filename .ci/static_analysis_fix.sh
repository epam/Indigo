#!/bin/sh -eux

# C++
# TODO
#mkdir build-clang-tidy
#cd build-clang-tidy
#cmake .. -DUSE_CLANG_TIDY=ON
#cd ..
#clang-tidy --version
#find . -not -path "./third_party/*" -a \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.c" \) -exec clang-tidy --fix --fix-errors -p build-clang-tidy --header-filter="*third-party*" {} +

clang-format --version
find . -not -path "./third_party/*" -a -not -path "./*build*/*" -a \( -iname "*.h" -o -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.c" \) -exec clang-format -i {} +

# Python

isort --version
black --version
pflake8 --version
for folder in api/http api/python bingo/bingo-elastic/python api/tests/integration utils/indigo-service/backend/service
do
  cd ${folder}
  isort .
  black .
  pflake8 .
  cd -
done

mypy --version
for folder in api/http api/python bingo/bingo-elastic/python utils/indigo-service/backend/service
do
    cd ${folder}
    export PYTHONPATH=${PWD}
    mypy --exclude=build .
    cd -
done
