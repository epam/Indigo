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
%EXEC_JAVAC% -cp ../../../../api/java/dist/indigo.jar;../../../../api/renderer/java/dist/indigo-renderer.jar com/epam/indigo/controls/*.java
%EXEC_JAR% cvf ../dist/common-controls.jar com/epam/indigo/controls/*.class com/epam/indigo/controls/images/*
del /Q com\epam\indigo\controls\*.class
cd ..
