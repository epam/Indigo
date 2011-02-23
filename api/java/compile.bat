@echo off
mkdir dist
cd src
javac -cp ../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
jar cvf ../dist/indigo-java.jar com/ggasoftware/indigo/*.class
del /Q com\ggasoftware\indigo\*.class
cd ..
