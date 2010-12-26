#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

cd ../api/jni
cd ../jni
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
cd ../../../legio/src

javac -cp ../../api/java/dist/indigo-java.jar:../../api/renderer/java/dist/indigo-renderer-java.jar com/gga/indigo/legio/*.java
jar cvfm ../legio.jar ../manifest.mf com/gga/indigo/legio/*.class
cd ..

name=legio-$version-linux

rm -rf $name $name.zip
mkdir -p $name/lib/Linux/x86
mkdir -p $name/lib/Linux/x64

cp LICENSE.GPL $name/
cp legio.jar $name/
cp ../api/java/dist/indigo-java.jar $name/lib/
cp ../api/renderer/java/dist/indigo-renderer-java.jar $name/lib/
cp ../api/jni/dist/Release32/GNU-Linux-x86/libindigo-jni.so $name/lib/Linux/x86
cp ../api/jni/dist/Release64/GNU-Linux-x86/libindigo-jni.so $name/lib/Linux/x64
cp ../api/renderer/jni/dist/Release32/GNU-Linux-x86/libindigo-renderer-jni.so $name/lib/Linux/x86
cp ../api/renderer/jni/dist/Release64/GNU-Linux-x86/libindigo-renderer-jni.so $name/lib/Linux/x64
cp legio.sh $name/legio
cp -r tests $name/

zip -r -9 $name.zip $name
