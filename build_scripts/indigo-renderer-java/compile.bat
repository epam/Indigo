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

set distPath=%CD%\dist
rd \S \Q %distPath%
md %distPath%
cd ..\..\api\renderer\java\src
echo %CD%
%EXEC_JAVAC% -cp ..\..\..\..\projects\indigo-java\dist\indigo.jar;..\..\..\..\common\jna\jna.jar com\ggasoftware\indigo\*.java
%EXEC_JAR% cvf %distPath%\indigo-renderer.jar com\ggasoftware\indigo\*.class
del /Q com\ggasoftware\indigo\*.class
cd %distPAth%\..

md com\ggasoftware\indigo\Win\x86
md com\ggasoftware\indigo\Win\x64

copy ..\indigo-renderer\dist\Win\x86\shared\Release\indigo-renderer.dll com\ggasoftware\indigo\Win\x86\
copy ..\indigo-renderer\dist\Win\x64\shared\Release\indigo-renderer.dll com\ggasoftware\indigo\Win\x64\

if "%ProgramFiles(x86)%" == "" goto L1
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" com\ggasoftware\indigo\Win\x86\
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll" com\ggasoftware\indigo\Win\x64\
goto L2
:L1
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" com\ggasoftware\indigo\Win\x86\
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll" com\ggasoftware\indigo\Win\x64\
:L2

%EXEC_JAR% uf dist\indigo-renderer.jar com\ggasoftware\indigo\Win\x86\indigo-renderer.dll
%EXEC_JAR% uf dist\indigo-renderer.jar com\ggasoftware\indigo\Win\x64\indigo-renderer.dll
%EXEC_JAR% uf dist\indigo-renderer.jar com\ggasoftware\indigo\Win\x86\msvcr100.dll
%EXEC_JAR% uf dist\indigo-renderer.jar com\ggasoftware\indigo\Win\x64\msvcr100.dll

rd /S /Q com
