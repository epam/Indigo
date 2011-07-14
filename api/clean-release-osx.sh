#!/bin/sh

for osxver in '10.5' '10.6'; do
   libdistr=indigo-libs-$version-osx$osxver
   xcodebuild -sdk macosx$osxver -configuration Release$osxver -alltargets clean
   cd renderer
   xcodebuild -sdk macosx$osxver -configuration Release$osxver -alltargets clean
   cd ..
done
