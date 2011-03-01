#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

echo 
echo ** BUILD API **
echo

javadistr=indigo-java-api-$version-osx
pythondistr=indigo-python-api-$version-osx

rm -rf ./$javadistr ./$javadistr.zip
mkdir $javadistr
rm -rf ./$pythondistr ./$pythondistr.zip
mkdir $pythondistr

cd java
./compile.sh
cd ../renderer/java
./compile.sh
cd ../..

cp LICENSE.GPL $pythondistr/
cp python/indigo.py $pythondistr/
cp renderer/python/indigo_renderer.py $pythondistr/

for osxver in '10.5' '10.6'; do
   libdistr=indigo-libs-$version-osx$osxver
   mkdir -p $libdistr
   mkdir -p $javadistr
   mkdir -p $pythondistr/lib/Mac/$osxver
   rm -rf build ../graph/build ../molecule/build ../reaction/build ../layout/build ../render2d/build ../tinyxml/build
   xcodebuild -sdk macosx$osxver -configuration Release -alltargets
   cd renderer
   rm -r build
   xcodebuild -sdk macosx$osxver -configuration Release -alltargets
   cd ..
   cp LICENSE.GPL $libdistr
   cp indigo.h $libdistr
   cp renderer/indigo-renderer.h $libdistr
   cp build/Release/libindigo.a $libdistr 
   cp build/Release/libindigo.dylib $libdistr 
   cp renderer/build/Release/libindigo-renderer.a $libdistr
   cp renderer/build/Release/libindigo-renderer.dylib $libdistr
   cp ../graph/build/Release/libgraph.a $libdistr 
   cp ../molecule/build/Release/libmolecule.a $libdistr 
   cp ../reaction/build/Release/libreaction.a $libdistr 
   cp ../layout/build/Release/liblayout.a $libdistr 
   cp ../render2d/build/Release/librender2d.a $libdistr 
   mkdir -p java/com/ggasoftware/indigo/Mac/$osxver
   cp build/Release/libindigo.dylib java/com/ggasoftware/indigo/Mac/$osxver/
   cp build/Release/libindigo.dylib $pythondistr/lib/Mac/$osxver
   mkdir -p renderer/java/com/ggasoftware/indigo/Mac/$osxver
   cp renderer/build/Release/libindigo-renderer.dylib renderer/java/com/ggasoftware/indigo/Mac/$osxver/
   cp renderer/build/Release/libindigo-renderer.dylib $pythondistr/lib/Mac/$osxver
   zip -r -9 $libdistr.zip $libdistr
done

cd java
./pack-libs-osx.sh
cd ../renderer/java
./pack-libs-osx.sh
cd ../..

cp LICENSE.GPL $javadistr/
cp java/dist/indigo.jar $javadistr
cp renderer/java/dist/indigo-renderer.jar $javadistr
cp ../common/jna/jna.jar $javadistr

zip -r -9 $javadistr.zip $javadistr
zip -r -9 $pythondistr.zip $pythondistr

