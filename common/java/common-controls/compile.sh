mkdir -p dist
cd src
javac -cp ../../../../api/java/dist/indigo.jar:../../../../api/renderer/java/dist/indigo-renderer.jar com/ggasoftware/indigo/controls/*.java
jar cvf ../dist/common-controls.jar com/ggasoftware/indigo/controls/*.class
rm -f com/ggasoftware/indigo/controls/*.class
cd ..

