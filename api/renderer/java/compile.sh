mkdir dist
cd src
javac -cp ../../../java/dist/indigo-java.jar com/gga/indigo/*.java
jar cvf ../dist/indigo-renderer-java.jar com/gga/indigo/*.class
cd ..
