#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

for osxver in '10.5' '10.6'; do
   xcodebuild -sdk macosx$osxver -configuration Release$osxver
   ./release-unix.sh indigo-depict-$version-osx-$osxver \
       build/Release$osxver/indigo-depict
done
