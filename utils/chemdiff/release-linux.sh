#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

cd ../../api
make CONF=ReleaseShared32
make CONF=ReleaseShared64
cd java
./compile.sh
./pack-libs-linux.sh
cd ../renderer
make CONF=ReleaseShared32
make CONF=ReleaseShared64
cd java
./compile.sh
./pack-libs-linux.sh
cd ../../../common/java/common-controls
./compile.sh
cd ../../../utils/chemdiff/src

javac -cp ../../../common/jna/jna.jar:../../../api/java/dist/indigo.jar:../../../api/renderer/java/dist/indigo-renderer.jar:../../../common/java/common-controls/dist/common-controls.jar com/ggasoftware/indigo/chemdiff/*.java
jar cvfm ../chemdiff.jar ../META-INF/manifest.mf ../META-INF/chemdiff-splash.png com/ggasoftware/indigo/chemdiff/*.class
cd ..

name=chemdiff-$version-linux

rm -rf $name $name.zip
mkdir -p $name/lib

cp LICENSE.GPL $name/
cp chemdiff.jar $name/
cp ../../common/jna/jna.jar $name/lib/
cp ../../api/java/dist/indigo.jar $name/lib/
cp ../../api/renderer/java/dist/indigo-renderer.jar $name/lib/
cp ../../common/java/common-controls/dist/common-controls.jar $name/lib/
cp chemdiff.sh $name/chemdiff
cp -r tests $name/

zip -r -9 $name.zip $name

