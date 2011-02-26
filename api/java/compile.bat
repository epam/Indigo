@echo off
mkdir dist
cd src
javac -cp ../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
jar cvf ../dist/indigo.jar com/ggasoftware/indigo/*.class
del /Q com\ggasoftware\indigo\*.class
cd ..

md com\ggasoftware\indigo\Win\x86
md com\ggasoftware\indigo\Win\x64

copy ..\dll\Win32\Release\indigo.dll com\ggasoftware\indigo\Win\x86\
copy ..\dll\x64\Release\indigo.dll com\ggasoftware\indigo\Win\x64\
copy ..\..\zlib-src\Win32\ReleaseDll\zlib.dll com\ggasoftware\indigo\Win\x86\
copy ..\..\zlib-src\x64\ReleaseDll\zlib.dll com\ggasoftware\indigo\Win\x64\

if "%ProgramFiles(x86)%" == "" goto L1
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" com\ggasoftware\indigo\Win\x86\
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll" com\ggasoftware\indigo\Win\x64\
goto L2
:L1
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" com\ggasoftware\indigo\Win\x86\
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll" com\ggasoftware\indigo\Win\x64\
:L2

jar uf dist/indigo.jar com\ggasoftware\indigo\Win\x86\indigo.dll
jar uf dist/indigo.jar com\ggasoftware\indigo\Win\x64\indigo.dll
jar uf dist/indigo.jar com\ggasoftware\indigo\Win\x86\zlib.dll
jar uf dist/indigo.jar com\ggasoftware\indigo\Win\x64\zlib.dll
jar uf dist/indigo.jar com\ggasoftware\indigo\Win\x86\msvcr100.dll
jar uf dist/indigo.jar com\ggasoftware\indigo\Win\x64\msvcr100.dll

rd /S /Q com
