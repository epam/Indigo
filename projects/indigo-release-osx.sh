#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

javadistr=indigo-java-api-$version-osx
pythondistr=indigo-python-api-$version-osx
apiPath=../api
rm -rf ./$javadistr ./$javadistr.zip
mkdir $javadistr
rm -rf ./$pythondistr ./$pythondistr.zip
mkdir $pythondistr

cp $apiPath/LICENSE.GPL $pythondistr/
cp $apiPath/python/indigo.py $pythondistr/
cp $apiPath/renderer/python/indigo_renderer.py $pythondistr/

for osxver in '10.5' '10.6'; do
   libdistr=indigo-libs-$version-osx$osxver
   mkdir -p $libdistr
   mkdir -p $javadistr
   mkdir -p $pythondistr/lib/Mac/$osxver
   cd indigo-renderer
   #rm CMakeCache.txt
   #xcodebuild clean
   cmake -G "Xcode"
   indigoRendererDistPath=$PWD/dist/Mac/$osxver/lib/Release
   xcodebuild -sdk macosx$osxver -configuration Release
   cd ..
   cp $apiPath/LICENSE.GPL $libdistr
   cp $apiPath/indigo.h $libdistr
   cp $apiPath/renderer/indigo-renderer.h $libdistr
   cp $indigoRendererDistPath/libindigo.a $libdistr 
   cp $indigoRendererDistPath/libindigo.dylib $libdistr 
   cp $indigoRendererDistPath/libindigo-renderer.a $libdistr
   cp $indigoRendererDistPath/libindigo-renderer.dylib $libdistr
   cp $indigoRendererDistPath/libgraph.a $libdistr 
   cp $indigoRendererDistPath/libmolecule.a $libdistr 
   cp $indigoRendererDistPath/libreaction.a $libdistr 
   cp $indigoRendererDistPath/liblayout.a $libdistr 
   cp $indigoRendererDistPath/librender2d.a $libdistr 
   cp $indigoRendererDistPath/libindigo.dylib $pythondistr/lib/Mac/$osxver
   cp $indigoRendererDistPath/libindigo-renderer.dylib $pythondistr/lib/Mac/$osxver
   zip -r -9 $libdistr.zip $libdistr
done

exit;
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

