mkdir dist
cd src
rm -rf com/ggasoftware/indigo/*.class
javac -cp ../../../java/dist/indigo-java.jar:../../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
jar cvf ../dist/indigo-renderer-java.jar com/ggasoftware/indigo/*.class
rm -rf com/ggasoftware/indigo/*.class
cd ..
