mkdir -p com/ggasoftware/indigo/Mac/10.5
mkdir -p com/ggasoftware/indigo/Mac/10.6
cp ../indigo-renderer/dist/Mac/10.5/lib/Release/libindigo.dylib com/ggasoftware/indigo/Mac/10.5/
cp ../indigo-renderer/dist/Mac/10.6/lib/Release/libindigo.dylib com/ggasoftware/indigo/Mac/10.6/
jar uf dist/indigo.jar com/ggasoftware/indigo/Mac/10.5/libindigo.dylib
jar uf dist/indigo.jar com/ggasoftware/indigo/Mac/10.6/libindigo.dylib
rm -rf com

