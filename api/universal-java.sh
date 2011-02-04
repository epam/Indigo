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
cp -r indigo-java-api-$1-osx/lib/* indigo-java-api-$1-universal/lib
cp -r indigo-java-api-$1-windows/lib/* indigo-java-api-$1-universal/lib
cp -r indigo-java-api-$1-linux/lib/* indigo-java-api-$1-universal/lib
zip -r -9 indigo-java-api-$1-universal.zip indigo-java-api-$1-universal
rm indigo-java-api-$1-windows.zip
rm indigo-java-api-$1-linux.zip
rm indigo-java-api-$1-osx.zip
rm indigo-java-api-$1-sun.zip
rm -r indigo-java-api-$1-windows
rm -r indigo-java-api-$1-osx
rm -r indigo-java-api-$1-linux
