rd /S /Q %1

mkdir %1

copy LICENSE.GPL %1\
copy indigo.h %1\
copy renderer\indigo-renderer.h %1\

mkdir %1\static
mkdir %1\dynamic

copy dll\%2\Release\indigo.dll %1\dynamic
copy dll\%2\Release\indigo.lib %1\dynamic

copy %2\Release\indigo.lib %1\static

copy renderer\%2\Release\indigo-renderer.lib %1\static
copy renderer\dll\%2\Release\indigo-renderer.dll %1\dynamic
copy renderer\dll\%2\Release\indigo-renderer.lib %1\dynamic

copy ..\graph\%2\Release\graph.lib %1\static
copy ..\molecule\%2\Release\molecule.lib %1\static
copy ..\reaction\%2\Release\reaction.lib %1\static
copy ..\layout\%2\Release\layout.lib %1\static
copy ..\render2d\%2\Release\render2d.lib %1\static

copy ..\zlib-src\%2\ReleaseDll\zlib.dll %1\dynamic

if "%ProgramFiles(x86)%" == "" goto L1
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\%3\Microsoft.VC100.CRT\msvcr100.dll" %1\dynamic
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\%3\Microsoft.VC100.CRT\msvcr100.dll" %1\dynamic
goto L2
:L1
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\%3\Microsoft.VC100.CRT\msvcr100.dll" %1\dynamic
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\%3\Microsoft.VC100.CRT\msvcr100.dll" %1\dynamic
:L2

zip -r -9 %1.zip %1
