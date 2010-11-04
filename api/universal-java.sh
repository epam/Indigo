if [ -z $1 ]; then
  echo "specify version";
  exit;
fi

wget http://scitouch.net/downloads/indigo-java-api-$1-windows.zip
wget http://scitouch.net/downloads/indigo-java-api-$1-linux.zip
wget http://scitouch.net/downloads/indigo-java-api-$1-osx.zip
unzip indigo-java-api-$1-windows.zip
unzip indigo-java-api-$1-linux.zip
unzip indigo-java-api-$1-osx.zip
mv indigo-java-api-$1-linux indigo-java-api-$1-universal
cp -r indigo-java-api-$1-osx/lib/* indigo-java-api-$1-universal/lib
cp -r indigo-java-api-$1-windows/lib/* indigo-java-api-$1-universal/lib
zip -r -9 indigo-java-api-$1-universal.zip indigo-java-api-$1-universal
rm indigo-java-api-$1-windows.zip
rm indigo-java-api-$1-linux.zip
rm indigo-java-api-$1-osx.zip
rm -r indigo-java-api-$1-windows
rm -r indigo-java-api-$1-osx
