mkdir -p dist
cd src
javac -cp ../../../../api/java/dist/indigo.jar:../../../../api/renderer/java/dist/indigo-renderer.jar com/epam/indigo/controls/*.java
jar cvf ../dist/common-controls.jar com/epam/indigo/controls/*.class com/epam/indigo/controls/images/*
rm -f com/epam/indigo/controls/*.class
cd ..

