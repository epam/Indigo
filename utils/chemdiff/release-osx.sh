#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

name=chemdiff-$version-osx

rm -rf $name $name.zip
mkdir -p $name/lib

cd ../../api/
for osxver in '10.5' '10.6'; do
  xcodebuild -sdk macosx$osxver -configuration Release$osxver -alltargets
  cd renderer
  xcodebuild -sdk macosx$osxver -configuration Release$osxver -alltargets 
  cd ..
done

cd java
./compile.sh
./pack-libs-osx.sh
cd ../renderer/java
./compile.sh
./pack-libs-osx.sh
cd ../../../common/java/common-controls
./compile.sh
cd ../../../utils/chemdiff/src

javac -cp ../../../api/java/dist/indigo.jar:../../../api/renderer/java/dist/indigo-renderer.jar:../../../common/java/common-controls/dist/common-controls.jar com/ggasoftware/indigo/chemdiff/*.java
jar cvfm ../chemdiff.jar ../manifest.mf com/ggasoftware/indigo/chemdiff/*.class
cd ..

cp LICENSE.GPL $name/
cp chemdiff.jar $name/
cp ../../common/jna/jna.jar $name/lib/
cp ../../api/java/dist/indigo.jar $name/lib/
cp ../../common/java/common-controls/dist/common-controls.jar $name/lib/
cp ../../api/renderer/java/dist/indigo-renderer.jar $name/lib/
cp chemdiff.sh $name/chemdiff
cp -r tests $name/

zip -r -9 $name.zip $name

