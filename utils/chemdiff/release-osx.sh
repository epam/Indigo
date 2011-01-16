#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

name=chemdiff-$version-osx

rm -rf $name $name.zip
mkdir -p $name/lib/Mac/10.5
mkdir -p $name/lib/Mac/10.6
cd ../../api/

for osxver in '10.5' '10.6'; do
  cd jni
  rm -rf build
  xcodebuild -sdk macosx$osxver -configuration Release -alltargets
  cp build/Release/libindigo-jni.dylib ../../utils/chemdiff/$name/lib/Mac/$osxver
  cd ../renderer/jni
  rm -rf build
  xcodebuild -sdk macosx$osxver -configuration Release -alltargets 
  cp build/Release/libindigo-renderer-jni.dylib ../../../utils/chemdiff/$name/lib/Mac/$osxver
  cd ../../
done

cd java
./compile.sh
cd ../renderer/java
./compile.sh
cd ../../../utils/chemdiff/src

javac -cp ../../../api/java/dist/indigo-java.jar:../../../api/renderer/java/dist/indigo-renderer-java.jar com/ggasoftware/indigo/chemdiff/*.java
jar cvfm ../chemdiff.jar ../manifest.mf com/ggasoftware/indigo/chemdiff/*.class
cd ..

cp LICENSE.GPL $name/
cp chemdiff.jar $name/
cp ../../api/java/dist/indigo-java.jar $name/lib/
cp ../../api/renderer/java/dist/indigo-renderer-java.jar $name/lib/
cp chemdiff.sh $name/chemdiff
cp -r tests $name/

zip -r -9 $name.zip $name

