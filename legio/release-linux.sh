#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

cd ../api/
make CONF=ReleaseShared32
make CONF=ReleaseShared64
cd java
chmod +x compile.sh
./compile.sh
cd ../renderer
make CONF=ReleaseShared32
make CONF=ReleaseShared64
cd java
chmod +x compile.sh
./compile.sh
cd ../../../legio/src

rm -rf com/ggasoftware/indigo/legio/*.class
javac -cp ../../api/java/dist/indigo-java.jar:../../api/renderer/java/dist/indigo-renderer-java.jar com/ggasoftware/indigo/legio/*.java
jar cvfm ../legio.jar ../manifest.mf com/ggasoftware/indigo/legio/*.class
rm -rf com/ggasoftware/indigo/legio/*.class
cd ..

name=legio-$version-linux

rm -rf $name $name.zip
mkdir -p $name/lib/Linux/x86
mkdir -p $name/lib/Linux/x64

cp LICENSE.GPL $name/
cp legio.jar $name/
cp ../common/jna/jna.jar $name/lib/
cp ../api/java/dist/indigo-java.jar $name/lib/
cp ../api/renderer/java/dist/indigo-renderer-java.jar $name/lib/
cp ../api/dist/ReleaseShared32/GNU-Linux-x86/libindigo.so $name/lib/Linux/x86
cp ../api/dist/ReleaseShared64/GNU-Linux-x86/libindigo.so $name/lib/Linux/x64
cp ../api/renderer/dist/ReleaseShared32/GNU-Linux-x86/libindigo-renderer.so $name/lib/Linux/x86
cp ../api/renderer/dist/ReleaseShared64/GNU-Linux-x86/libindigo-renderer.so $name/lib/Linux/x64
cp legio.sh $name/legio
cp -r tests $name/

zip -r -9 $name.zip $name

