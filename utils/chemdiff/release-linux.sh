#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

cd ../../api/jni
make CONF=Release32
make CONF=Release64
cd ../java
chmod +x compile.sh
./compile.sh
cd ../renderer/jni
make CONF=Release32
make CONF=Release64
cd ../java
chmod +x compile.sh
./compile.sh
cd ../../../utils/chemdiff/src

javac -cp ../../../api/java/dist/indigo-java.jar:../../../api/renderer/java/dist/indigo-renderer-java.jar com/gga/indigo/chemdiff/*.java
jar cvfm ../chemdiff.jar ../manifest.mf com/gga/indigo/chemdiff/*.class
cd ..

name=chemdiff-$version-linux

rm -rf $name $name.zip
mkdir -p $name/lib/Linux/x86
mkdir -p $name/lib/Linux/x64

cp LICENSE.GPL $name/
cp chemdiff.jar $name/
cp ../../api/java/dist/indigo-java.jar $name/lib/
cp ../../api/renderer/java/dist/indigo-renderer-java.jar $name/lib/
cp ../../api/jni/dist/Release32/GNU-Linux-x86/libindigo-jni.so $name/lib/Linux/x86
cp ../../api/jni/dist/Release64/GNU-Linux-x86/libindigo-jni.so $name/lib/Linux/x64
cp ../../api/renderer/jni/dist/Release32/GNU-Linux-x86/libindigo-renderer-jni.so $name/lib/Linux/x86
cp ../../api/renderer/jni/dist/Release64/GNU-Linux-x86/libindigo-renderer-jni.so $name/lib/Linux/x64
cp chemdiff.sh $name/chemdiff

zip -r -9 $name.zip $name
