#!/bin/sh

name=$1

if [ -z $name ]; then
  echo "specify name";
  exit;
fi

rm -rf ./$name 

mkdir -p $name/lib/Linux/x86
mkdir -p $name/lib/Linux/x64

cp LICENSE.GPL $name/
cp java/dist/indigo-java.jar $name/
cp renderer/java/dist/indigo-renderer-java.jar $name/
cp ../common/jna/jna.jar $name/
cp dist/ReleaseShared32/GNU-Linux-x86/libindigo.so $name/lib/Linux/x86
cp dist/ReleaseShared64/GNU-Linux-x86/libindigo.so $name/lib/Linux/x64
cp renderer/dist/ReleaseShared32/GNU-Linux-x86/libindigo-renderer.so $name/lib/Linux/x86
cp renderer/dist/ReleaseShared64/GNU-Linux-x86/libindigo-renderer.so $name/lib/Linux/x64

zip -r -9 $name.zip $name
