#!/usr/bin/env bash

set -euo pipefail

if [ -z "$IN_NIX_SHELL" ]; then
  echo "Activate the nix flake first"
  exit 1
fi

CLEAN=false

while [ "$#" -gt 0 ]; do
  case "$1" in
    --clean) CLEAN=true; shift;;
    *) exit 1;
  esac
done

echo "⚙️ Installing python requirements..."
python3 -m pip install -r requirements.txt

if [ "$CLEAN" = true ]; then
  rm -rf build dist || true
fi

mkdir -p build
cd build
echo "⚙️ Configuring and building C++ library..."
cmake .. \
  -DBUILD_INDIGO=ON \
  -DBUILD_INDIGO_WRAPPERS=ON \
  -DBUILD_INDIGO_UTILS=ON \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_FLAGS="-Wno-implicit-int -DINDIGO_DEBUG" \
  -DCMAKE_CXX_FLAGS="-DINDIGO_DEBUG"
cmake --build . --config Debug --target indigo-python -j10

cd ..
python3 -m pip install --upgrade --force-reinstall dist/*indigo*.whl
VERSION=$(python3 -c 'import indigo; print(indigo.__version__)')
echo "✨ Built and installed indigo version $VERSION"
