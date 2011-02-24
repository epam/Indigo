@echo off
mkdir dist
cd src
javac -cp ../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
jar cvf ../dist/indigo.jar com/ggasoftware/indigo/*.class
del /Q com\ggasoftware\indigo\*.class
cd ..

md com\ggasoftware\indigo\Win\x86
md com\ggasoftware\indigo\Win\x64

copy ..\dll\Win32\Release\indigo.dll com\ggasoftware\indigo\Win\x86
copy ..\dll\x64\Release\indigo.dll com\ggasoftware\indigo\Win\x64
copy ..\..\zlib-src\Win32\ReleaseDll\zlib.dll com\ggasoftware\indigo\Win\x86
copy ..\..\zlib-src\x64\ReleaseDll\zlib.dll com\ggasoftware\indigo\Win\x64

jar uf dist/indigo.jar com\ggasoftware\indigo\Win\x86\indigo.dll
jar uf dist/indigo.jar com\ggasoftware\indigo\Win\x64\indigo.dll
jar uf dist/indigo.jar com\ggasoftware\indigo\Win\x86\zlib.dll
jar uf dist/indigo.jar com\ggasoftware\indigo\Win\x64\zlib.dll

rd /S /Q com
