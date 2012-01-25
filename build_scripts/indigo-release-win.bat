@echo off
@rem Set variables
set version=%1
if [%1] == [] echo Specify version! && exit
set indigoAPIPath=%CD%\..\api
set javadistr=indigo-java-api-%version%-win
rd /S /Q %javadistr%
mkdir %javadistr%
set pythondistr=indigo-python-api-%version%-win
rd /S /Q %pythondistr%
mkdir %pythondistr%
set libdistr=indigo-libs-%version%-win32
rd /S /Q %libdistr%
mkdir %libdistr%
cd indigo-renderer
@rem Build Win32 libs
@rem del CMakeCache.txt
@rem cmake . -DSUBSYSTEM_FOLDER_NAME=x86
@rem cmake --build . --config Release
@rem Copy Win32 libs
cd ..
copy %indigoAPIPath%\indigo.h %libdistr%
copy %indigoAPIPath%\renderer\indigo-renderer.h %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\z.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\common.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\tinyxml.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\graph.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\molecule.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\reaction.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\layout.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\indigo.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\png.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\pixman.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\cairo.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\render2d.lib %libdistr%
copy indigo-renderer\dist\Win\x86\lib\Release\indigo-renderer.lib %libdistr%
copy indigo-renderer\dist\Win\x86\shared\Release\indigo.dll %libdistr%
copy indigo-renderer\dist\Win\x86\shared\Release\indigo-renderer.dll %libdistr%
mkdir %pythondistr%\lib\Win\x86\
copy indigo-renderer\dist\Win\x86\shared\Release\indigo.dll %pythondistr%\lib\Win\x86\
copy indigo-renderer\dist\Win\x86\shared\Release\indigo-renderer.dll %pythondistr%\lib\Win\x86\
zip -r -9 %libdistr%.zip %libdistr%
@rem Build Win64 libs
set libdistr=indigo-libs-%version%-win64
rd /S /Q %libdistr%
mkdir %libdistr%
cd indigo-renderer
@rem del CMakeCache.txt
@rem cmake . -G "Visual Studio 10 Win64" -DSUBSYSTEM_FOLDER_NAME=x64
@rem cmake --build . --config Release
@rem Copy Win64 libs
cd ..
copy %indigoAPIPath%\indigo.h %libdistr%
copy %indigoAPIPath%\renderer\indigo-renderer.h %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\z.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\common.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\tinyxml.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\graph.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\molecule.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\reaction.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\layout.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\indigo.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\png.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\pixman.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\cairo.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\render2d.lib %libdistr%
copy indigo-renderer\dist\Win\x64\lib\Release\indigo-renderer.lib %libdistr%
copy indigo-renderer\dist\Win\x64\shared\Release\indigo.dll %libdistr%
copy indigo-renderer\dist\Win\x64\shared\Release\indigo-renderer.dll %libdistr%
mkdir %pythondistr%\lib\Win\x64\
copy indigo-renderer\dist\Win\x64\shared\Release\indigo.dll %pythondistr%\lib\Win\x64\
copy indigo-renderer\dist\Win\x64\shared\Release\indigo-renderer.dll %pythondistr%\lib\Win\x64\
zip -r -9 %libdistr%.zip %libdistr%

@rem Process the python archive
copy ..\api\python\indigo.py %pythondistr%
copy ..\api\renderer\python\indigo_renderer.py %pythondistr%
copy %indigoAPIPath%\LICENSE.GPL %pythondistr%
zip -r -9 %pythondistr%.zip %pythondistr%

@rem Process the java archive
cd indigo-java
call compile.bat
cd ..\indigo-renderer-java
call compile.bat
cd ..
copy %indigoAPIPath%\LICENSE.GPL %javadistr%
copy indigo-java\dist\indigo.jar %javadistr%
copy indigo-renderer-java\dist\indigo-renderer.jar %javadistr%
zip -r -9 %javadistr%.zip %javadistr%
