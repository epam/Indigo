mkdir dist
cd src
rm -rf com/ggasoftware/indigo/*.class
javac -cp ../../../java/dist/indigo.jar:../../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
jar cvf ../dist/indigo-renderer.jar com/ggasoftware/indigo/*.class
rm -rf com/ggasoftware/indigo/*.class
cd ..

