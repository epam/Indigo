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
%EXEC_JAVAC% -cp ../../../java/dist/indigo.jar;../../../../common/jna/jna.jar com/ggasoftware/indigo/*.java
%EXEC_JAR% cvf ../dist/indigo-renderer.jar com/ggasoftware/indigo/*.class
del /Q com\ggasoftware\indigo\*.class
cd ..

md com\ggasoftware\indigo\Win\x86
md com\ggasoftware\indigo\Win\x64

copy ..\dll\Win32\Release\indigo-renderer.dll com\ggasoftware\indigo\Win\x86
copy ..\dll\x64\Release\indigo-renderer.dll com\ggasoftware\indigo\Win\x64

%EXEC_JAR% uf dist/indigo-renderer.jar com\ggasoftware\indigo\Win\x86\indigo-renderer.dll
%EXEC_JAR% uf dist/indigo-renderer.jar com\ggasoftware\indigo\Win\x64\indigo-renderer.dll

rd /S /Q com
