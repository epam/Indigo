rd /S /Q %1

mkdir %1\lib\Win\x86
mkdir %1\lib\Win\x64

copy LICENSE.GPL %1\
copy python\indigo.py %1\
copy renderer\python\indigo_renderer.py %1\
copy dll\Win32\Release\indigo.dll %1\lib\Win\x86\
copy dll\x64\Release\indigo.dll %1\lib\Win\x64\
copy renderer\dll\Win32\Release\indigo-renderer.dll %1\lib\Win\x86\
copy renderer\dll\x64\Release\indigo-renderer.dll %1\lib\Win\x64\
copy ..\zlib-src\Win32\ReleaseDll\zlib.dll %1\lib\Win\x86\
copy ..\zlib-src\x64\ReleaseDll\zlib.dll %1\lib\Win\x64\

if "%ProgramFiles(x86)%" == "" goto L1
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" %1\lib\Win\x86\
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll" %1\lib\Win\x64\
goto L2
:L1
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" %1\lib\Win\x86\
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll" %1\lib\Win\x64\
:L2


zip -r -9 %1.zip %1
