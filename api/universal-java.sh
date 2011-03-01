#!/bin/bash

if [ -z $1 ]; then
  echo "specify version";
  exit;
fi

wget -c http://ggasoftware.com/downloads/indigo-java-api-$1-windows.zip
wget -c http://ggasoftware.com/downloads/indigo-java-api-$1-linux.zip
wget -c http://ggasoftware.com/downloads/indigo-java-api-$1-osx.zip
wget -c http://ggasoftware.com/downloads/indigo-java-api-$1-sun.zip
rm -rf indigo-java-api-$1-{windows,linux,osx,sun,universal}
unzip indigo-java-api-$1-windows.zip
unzip indigo-java-api-$1-linux.zip
unzip indigo-java-api-$1-osx.zip
unzip indigo-java-api-$1-sun.zip
mv indigo-java-api-$1-sun indigo-java-api-$1-universal
cd indigo-java-api-$1-universal
jar xf ../indigo-java-api-$1-windows/indigo.jar \
    com/ggasoftware/indigo/Win/x86/indigo.dll \
    com/ggasoftware/indigo/Win/x86/msvcr100.dll \
    com/ggasoftware/indigo/Win/x86/zlib.dll \
    com/ggasoftware/indigo/Win/x64/indigo.dll \
    com/ggasoftware/indigo/Win/x64/msvcr100.dll \
    com/ggasoftware/indigo/Win/x64/zlib.dll
jar xf ../indigo-java-api-$1-linux/indigo.jar \
    com/ggasoftware/indigo/Linux/x86/libindigo.so \
    com/ggasoftware/indigo/Linux/x64/libindigo.so
jar xf ../indigo-java-api-$1-osx/indigo.jar \
    com/ggasoftware/indigo/Mac/10.5/libindigo.dylib \
    com/ggasoftware/indigo/Mac/10.6/libindigo.dylib
jar uf indigo.jar com
rm -r com
jar xf ../indigo-java-api-$1-windows/indigo-renderer.jar \
    com/ggasoftware/indigo/Win/x86/indigo-renderer.dll \
    com/ggasoftware/indigo/Win/x64/indigo-renderer.dll
jar xf ../indigo-java-api-$1-linux/indigo-renderer.jar \
    com/ggasoftware/indigo/Linux/x86/libindigo-renderer.so \
    com/ggasoftware/indigo/Linux/x64/libindigo-renderer.so
jar xf ../indigo-java-api-$1-osx/indigo-renderer.jar \
    com/ggasoftware/indigo/Mac/10.5/libindigo-renderer.dylib \
    com/ggasoftware/indigo/Mac/10.6/libindigo-renderer.dylib
jar uf indigo-renderer.jar com
rm -r com

cd ..    
rm -f indigo-java-api-$1-universal.zip
zip -r -9 indigo-java-api-$1-universal.zip indigo-java-api-$1-universal
rm indigo-java-api-$1-windows.zip
rm indigo-java-api-$1-linux.zip
rm indigo-java-api-$1-osx.zip
rm indigo-java-api-$1-sun.zip
rm -r indigo-java-api-$1-windows
rm -r indigo-java-api-$1-osx
rm -r indigo-java-api-$1-linux
