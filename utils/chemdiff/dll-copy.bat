md lib\Win\x86
md lib\Win\x64

cd ..\..\api

if "%ProgramFiles(x86)%" == "" goto L1
call "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll" lib\Win\x64\
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" lib\Win\x64\
goto L2
:L1
call "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" lib\Win\x86\
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" lib\Win\x86\
:L2
devenv indigo-api.sln /build "ReleaseDLL|x64"
devenv indigo-api.sln /build "ReleaseDLL|Win32"

cd ..\utils\chemdiff
copy ..\..\api\dll\Win32\Release\indigo.dll lib\Win\x86\
copy ..\..\api\renderer\dll\Win32\Release\indigo-renderer.dll lib\Win\x86\
copy ..\..\zlib-src\Win32\ReleaseDll\zlib.dll lib\Win\x86\

copy ..\..\api\dll\x64\Release\indigo.dll lib\Win\x64\
copy ..\..\api\renderer\dll\x64\Release\indigo-renderer.dll lib\Win\x64\
copy ..\..\zlib-src\x64\ReleaseDll\zlib.dll lib\Win\x64\