#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

name=legio-$version-osx

rm -rf $name $name.zip
mkdir -p $name/lib/Mac/10.5
mkdir -p $name/lib/Mac/10.6
cd ../api/

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
cd ../../../legio/src

javac -cp ../../api/java/dist/indigo.jar:../../api/renderer/java/dist/indigo-renderer.jar com/ggasoftware/indigo/legio/*.java
jar cvfm ../legio.jar ../manifest.mf com/ggasoftware/indigo/legio/*.class
cd ..

cp LICENSE.GPL $name/
cp legio.jar $name/
cp ../common/jna/jna.jar $name/lib/
cp ../api/java/dist/indigo.jar $name/lib/
cp ../api/renderer/java/dist/indigo-renderer.jar $name/lib/
cp legio.sh $name/legio
cp -r tests $name/

zip -r -9 $name.zip $name
