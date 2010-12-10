if [ -z $1 ]; then
  echo "specify version";
  exit;
fi

wget http://ggasoftware.com/downloads/indigo-python-api-$1-windows.zip
wget http://ggasoftware.com/downloads/indigo-python-api-$1-linux.zip
wget http://ggasoftware.com/downloads/indigo-python-api-$1-osx.zip
unzip indigo-python-api-$1-windows.zip
unzip indigo-python-api-$1-linux.zip
unzip indigo-python-api-$1-osx.zip
mv indigo-python-api-$1-linux indigo-python-api-$1-universal
cp -r indigo-python-api-$1-osx/lib/* indigo-python-api-$1-universal/lib
cp -r indigo-python-api-$1-windows/lib/* indigo-python-api-$1-universal/lib
zip -r -9 indigo-python-api-$1-universal.zip indigo-python-api-$1-universal
rm indigo-python-api-$1-windows.zip
rm indigo-python-api-$1-linux.zip
rm indigo-python-api-$1-osx.zip
rm -r indigo-python-api-$1-windows
rm -r indigo-python-api-$1-osx
