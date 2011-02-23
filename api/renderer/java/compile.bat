@echo off
mkdir dist
cd src
javac -cp ../../../java/dist/indigo-java.jar;../../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
jar cvf ../dist/indigo-renderer-java.jar com/ggasoftware/indigo/*.class
del /Q com\ggasoftware\indigo\*.class
cd ..
