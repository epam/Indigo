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
  rm -rf ../graph.build ../molecule/build ../layout/build ../reaction/build ../tinyxml/build ../render2d/build ../api/build ../api/renderer/build
  cd jni
  rm -rf build
  xcodebuild -sdk macosx$osxver -configuration Release -alltargets
  cp build/Release/libindigo-jni.dylib ../../legio/$name/lib/Mac/$osxver
  cd ../renderer/jni
  rm -rf build
  xcodebuild -sdk macosx$osxver -configuration Release -alltargets 
  cp build/Release/libindigo-renderer-jni.dylib ../../../legio/$name/lib/Mac/$osxver
  cd ../../
done

cd java
chmod +x compile.sh
./compile.sh
cd ../renderer/java
chmod +x compile.sh
./compile.sh
cd ../../../legio/src

javac -cp ../../api/java/dist/indigo-java.jar:../../api/renderer/java/dist/indigo-renderer-java.jar com/ggasoftware/indigo/legio/*.java
jar cvfm ../legio.jar ../manifest.mf com/ggasoftware/indigo/legio/*.class
cd ..


cp LICENSE.GPL $name/
cp legio.jar $name/
cp ../api/java/dist/indigo-java.jar $name/lib/
cp ../api/renderer/java/dist/indigo-renderer-java.jar $name/lib/
cp legio.sh $name/legio
cp -r tests $name/

zip -r -9 $name.zip $name
