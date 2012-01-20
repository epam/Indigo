mkdir -p com/ggasoftware/indigo/Mac/10.5
mkdir -p com/ggasoftware/indigo/Mac/10.6
cp ../indigo/dist/Linux/x86/lib/Release/libindigo.dylib com/ggasoftware/indigo/Linux/x86/
cp ../indigo/dist/Linux/x64/lib/Release/libindigo.dylib com/ggasoftware/indigo/Linux/x64/
jar uf dist/indigo.jar com/ggasoftware/indigo/Linux/x86/libindigo.dylib
jar uf dist/indigo.jar com/ggasoftware/indigo/Linux/x64/libindigo.dylib
rm -rf com

