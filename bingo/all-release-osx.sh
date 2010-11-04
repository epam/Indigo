#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

for osxver in '10.5' '10.6'; do
   rm -rf build
   xcodebuild -sdk macosx$osxver -configuration Release
   ./bingo-release.sh bingo-oracle-$version-osx-$osxver-x86_64 \
       build/Release/libbingo.dylib
   rm -rf bingo-oracle-$version-osx-$osxver-x86_64
done
