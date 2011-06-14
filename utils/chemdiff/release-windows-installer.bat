@echo off
if "%1" == "" goto NOVER
mkdir lib
rem mkdir lib\Win\x86
rem mkdir lib\Win\x64

if "%ProgramFiles(x86)%" == "" goto L1
set MAKENSIS="%ProgramFiles(x86)%\NSIS\makensis.exe"
goto L2
:L1
set MAKENSIS="%ProgramFiles%\NSIS\makensis.exe"
:L2

cd ..\..\api\java
call compile.bat
cd ..\renderer\java
call compile.bat
cd ..\..\..\common\java\common-controls
call compile.bat
cd ..\..\..\utils\chemdiff

copy ..\..\common\jna\jna.jar lib\
copy ..\..\common\java\common-controls\dist\common-controls.jar lib\
copy ..\..\api\java\dist\indigo.jar lib\
copy ..\..\api\renderer\java\dist\indigo-renderer.jar lib\
cd src
javac -cp ..\lib\indigo.jar;..\lib\indigo-renderer.jar;..\lib\common-controls.jar com/ggasoftware/indigo/chemdiff/*.java
jar cvfm ..\chemdiff.jar ../META-INF/manifest.mf com/ggasoftware/indigo/chemdiff/*.class
del /Q com/ggasoftware/indigo/chemdiff/*.class
cd ..
jar -uf chemdiff.jar META-INF/chemdiff-splash.png

rem call dll-copy.bat
echo start javaw -jar -Xss10m %%0\..\chemdiff.jar > launch.bat

%MAKENSIS% /DVERSION=%1 chemdiff_installer.nsi 
goto END

:NOVER
echo Specify version please
:END
