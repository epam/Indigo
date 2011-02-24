mkdir dist
cd src
rm -rf com/ggasoftware/indigo/*.class
javac -cp ../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
jar cvf ../dist/indigo.jar com/ggasoftware/indigo/*.class
rm -rf com/ggasoftware/indigo/*.class
cd ..

mkdir -p com/ggasoftware/indigo/Linux/x86
mkdir -p com/ggasoftware/indigo/Linux/x64

cp ../dist/ReleaseShared32/GNU-Linux-x86/libindigo.so com/ggasoftware/indigo/Linux/x86
cp ../dist/ReleaseShared64/GNU-Linux-x86/libindigo.so com/ggasoftware/indigo/Linux/x64

jar uf dist/indigo.jar com/ggasoftware/indigo/Linux/x86/libindigo.so 
jar uf dist/indigo.jar com/ggasoftware/indigo/Linux/x64/libindigo.so 

rm com/ggasoftware/indigo/Linux/x86/libindigo.so
rm com/ggasoftware/indigo/Linux/x64/libindigo.so
