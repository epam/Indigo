#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

for osxver in '10.5' '10.6'; do
   rm -rf build ../../graph.build ../../molecule/build ../../layout/build ../../reaction/build ../../tinyxml/build ../../render2d/build ../../api/build ../../api/renderer/build
   xcodebuild -sdk macosx$osxver -configuration Release
   ./release-unix.sh indigo-depict-$version-osx-$osxver \
       build/Release/indigo-depict
done
