mkdir dist
cd src
javac com/ggasoftware/indigo/*.java
jar cvf ../dist/indigo-java.jar com/ggasoftware/indigo/*.class
cd ..
