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
   indigoRendererDistPath=$PWD/dist/Mac/$osxver/lib/Release
   #rm CMakeCache.txt
   #cmake -G Xcode -DSUBSYSTEM_FOLDER_NAME=$osxver
   #cmake --build . --config Release --clean-first -- -sdk macosx$osxver
   #cmake --build . --config Release -- -sdk macosx$osxver
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

cd indigo-java
./compile.sh
./pack-libs-osx.sh
cd ../indigo-renderer-java
./compile.sh
./pack-libs-osx.sh
cd ..

cp $apiPath/LICENSE.GPL $javadistr/
cp indigo-java/dist/indigo.jar $javadistr
cp indigo-renderer-java/dist/indigo-renderer.jar $javadistr
cp ../../common/jna/jna.jar $javadistr

zip -r -9 $javadistr.zip $javadistr
zip -r -9 $pythondistr.zip $pythondistr

