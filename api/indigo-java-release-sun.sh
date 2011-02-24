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
cp ../common/jna/jna.jar $name/
cp java/dist/indigo-java.jar $name/
cp renderer/java/dist/indigo-renderer-java.jar $name/
cp sparc32shared/libindigo.so $name/lib/Sun/sparc32/
cp sparc64shared/libindigo.so $name/lib/Sun/sparc64/

zip -r -9 $name.zip $name
