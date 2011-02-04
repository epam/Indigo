#!/bin/sh

name=$1

if [ -z $name ]; then
  echo "specify name";
  exit;
fi

rm -rf ./$name 

mkdir -p $name/lib/Sun/sparc32
mkdir -p $name/lib/Sun/sparc64

cp LICENSE.GPL $name/
cp java/dist/indigo-java.jar $name/
cp renderer/java/dist/indigo-renderer-java.jar $name/
cp jni/sparc32/libindigo-jni.so $name/lib/Sun/sparc32/
cp jni/sparc64/libindigo-jni.so $name/lib/Sun/sparc64/

zip -r -9 $name.zip $name
