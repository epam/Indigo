mkdir -p com/ggasoftware/indigo/Mac/10.5
mkdir -p com/ggasoftware/indigo/Mac/10.6
cp ../indigo/dist/Linux/x86/lib/Release/libindigo-renderer.dylib com/ggasoftware/indigo/Linux/x86/
cp ../indigo/dist/Linux/x64/lib/Release/libindigo-renderer.dylib com/ggasoftware/indigo/Linux/x64/
jar uf dist/indigo.jar com/ggasoftware/indigo/Linux/x86/libindigo-renderer.dylib
jar uf dist/indigo.jar com/ggasoftware/indigo/Linux/x64/libindigo-renderer.dylib
rm -rf com

