#!/bin/sh

version=$1

if [ -z $version ]; then
  echo "specify version";
  exit;
fi

name=chemdiff-$version

rm -rf $name $name.zip
mkdir -p $name/examples

rm -f indigo-java-api-$version-universal.zip
rm -rf indigo-java-api-$version-universal
wget http://ggasoftware.com/downloads/indigo-java-api-$version-universal.zip
unzip indigo-java-api-$version-universal.zip 

cp -r indigo-java-api-$version-universal/lib $name/
cp ../../api/LICENSE.GPL $name/
cp dist/README.TXT $name/
cp dist/chemdiff.jar $name/
cp ../../api/java/dist/indigo-java.jar $name/lib/

zip -r -9 $name.zip $name
