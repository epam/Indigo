@echo off

if "%JAVA_HOME%"=="" goto jDef1
goto jDef2
:jDef1
echo JAVA_HOME is not defined will use 'javac' and 'jar' commands
set EXEC_JAVAC=javac
set EXEC_JAR=jar
goto jDefEx
:jDef2
set EXEC_JAVAC="%JAVA_HOME%\bin\javac"
set EXEC_JAR="%JAVA_HOME%\bin\jar"
:jDefEx

mkdir dist
cd src
%EXEC_JAVAC% -cp ../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
%EXEC_JAR% cvf ../dist/indigo.jar com/ggasoftware/indigo/*.class
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

%EXEC_JAR% uf dist/indigo.jar com\ggasoftware\indigo\Win\x86\indigo.dll
%EXEC_JAR% uf dist/indigo.jar com\ggasoftware\indigo\Win\x64\indigo.dll
%EXEC_JAR% uf dist/indigo.jar com\ggasoftware\indigo\Win\x86\zlib.dll
%EXEC_JAR% uf dist/indigo.jar com\ggasoftware\indigo\Win\x64\zlib.dll
%EXEC_JAR% uf dist/indigo.jar com\ggasoftware\indigo\Win\x86\msvcr100.dll
%EXEC_JAR% uf dist/indigo.jar com\ggasoftware\indigo\Win\x64\msvcr100.dll

rd /S /Q com
