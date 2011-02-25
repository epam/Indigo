mkdir dist
cd src
rm -rf com/ggasoftware/indigo/*.class
javac -cp ../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
jar cvf ../dist/indigo.jar com/ggasoftware/indigo/*.class
rm -rf com/ggasoftware/indigo/*.class
cd ..

