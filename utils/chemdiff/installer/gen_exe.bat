@echo off
if "%ProgramFiles(x86)%" == "" goto L1
set MAKENSIS="%ProgramFiles(x86)%\NSIS\makensis.exe"
goto L2
:L1
set MAKENSIS="%ProgramFiles%\NSIS\makensis.exe"
:L2

cd ..\

echo start javaw -jar -Xss10m chemdiff.jar > dist\launch.bat

mkdir dist\lib
copy ..\..\api\java\dist\indigo-java.jar dist\lib\
copy ..\..\api\renderer\java\dist\indigo-renderer-java.jar dist\lib\

%MAKENSIS% installer\chemdiff_installer.nsi

cd installer
