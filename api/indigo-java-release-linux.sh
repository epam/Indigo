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
cp jni/dist/Release32/GNU-Linux-x86/libindigo-jni.so $name/lib/Linux/x86
cp jni/dist/Release64/GNU-Linux-x86/libindigo-jni.so $name/lib/Linux/x64
cp renderer/jni/dist/Release32/GNU-Linux-x86/libindigo-renderer-jni.so $name/lib/Linux/x86
cp renderer/jni/dist/Release64/GNU-Linux-x86/libindigo-renderer-jni.so $name/lib/Linux/x64

zip -r -9 $name.zip $name
