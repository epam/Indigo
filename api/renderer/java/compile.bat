@echo off
mkdir dist
cd src
javac -cp ../../../java/dist/indigo.jar;../../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
jar cvf ../dist/indigo-renderer.jar com/ggasoftware/indigo/*.class
del /Q com\ggasoftware\indigo\*.class
cd ..

md com\ggasoftware\indigo\Win\x86
md com\ggasoftware\indigo\Win\x64

copy ..\dll\Win32\Release\indigo-renderer.dll com\ggasoftware\indigo\Win\x86
copy ..\dll\x64\Release\indigo-renderer.dll com\ggasoftware\indigo\Win\x64

jar uf dist/indigo-renderer.jar com\ggasoftware\indigo\Win\x86\indigo-renderer.dll
jar uf dist/indigo-renderer.jar com\ggasoftware\indigo\Win\x64\indigo-renderer.dll

rd /S /Q com
