mkdir -p com/ggasoftware/indigo/Sun/sparc32
mkdir -p com/ggasoftware/indigo/Sun/sparc64

cp ../sparc32shared/libindigo.so com/ggasoftware/indigo/Sun/sparc32
cp ../sparc64shared/libindigo.so com/ggasoftware/indigo/Sun/sparc64

jar uf dist/indigo.jar com/ggasoftware/indigo/Sun/sparc32
jar uf dist/indigo.jar com/ggasoftware/indigo/Sun/sparc64

rm -rf com
