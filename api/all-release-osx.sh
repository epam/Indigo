#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

javadistr=build_release/indigo-java-api-$version-osx
pythondistr=build_release/indigo-python-api-$version-osx

rm -rf ./$javadistr ./$javadistr.zip
mkdir $javadistr
rm -rf ./$pythondistr ./$pythondistr.zip
mkdir $pythondistr

cp LICENSE.GPL $pythondistr/
cp python/indigo.py $pythondistr/
cp renderer/python/indigo_renderer.py $pythondistr/

for osxver in '10.5' '10.6'; do
   libdistr=build_release/indigo-libs-$version-osx$osxver
   mkdir -p $libdistr
   mkdir -p $javadistr
   mkdir -p $pythondistr/lib/Mac/$osxver
   xcodebuild -sdk macosx$osxver -configuration Release$osxver -alltargets
   cd renderer
   xcodebuild -sdk macosx$osxver -configuration Release$osxver -alltargets
   cd ..
   cp LICENSE.GPL $libdistr
   cp indigo.h $libdistr
   cp renderer/indigo-renderer.h $libdistr
   cp build/Release$osxver/libindigo.a $libdistr 
   cp build/Release$osxver/libindigo.dylib $libdistr 
   cp renderer/build/Release$osxver/libindigo-renderer.a $libdistr
   cp renderer/build/Release$osxver/libindigo-renderer.dylib $libdistr
   cp ../graph/build/Release$osxver/libgraph.a $libdistr 
   cp ../molecule/build/Release$osxver/libmolecule.a $libdistr 
   cp ../reaction/build/Release$osxver/libreaction.a $libdistr 
   cp ../layout/build/Release$osxver/liblayout.a $libdistr 
   cp ../render2d/build/Release$osxver/librender2d.a $libdistr 
   cp build/Release$osxver/libindigo.dylib $pythondistr/lib/Mac/$osxver
   cp renderer/build/Release$osxver/libindigo-renderer.dylib $pythondistr/lib/Mac/$osxver
   zip -r -9 $libdistr.zip $libdistr
done

cd java
./compile.sh
./pack-libs-osx.sh
cd ../renderer/java
./compile.sh
./pack-libs-osx.sh
cd ../..

cp LICENSE.GPL $javadistr/
cp java/dist/indigo.jar $javadistr
cp renderer/java/dist/indigo-renderer.jar $javadistr
cp ../common/jna/jna.jar $javadistr

zip -r -9 $javadistr.zip $javadistr
zip -r -9 $pythondistr.zip $pythondistr

