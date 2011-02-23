md lib\Win\x86
md lib\Win\x64

cd ..\..\api

if "%ProgramFiles(x86)%" == "" goto L1
call "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
goto L2
:L1
call "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
:L2
devenv indigo-api.sln /build "DebugDLL|x64"
devenv indigo-api.sln /build "DebugDLL|Win32"

cd ..\utils\chemdiff
copy ..\..\api\dll\Win32\Debug\indigo.dll lib\Win\x86\
copy ..\..\api\renderer\dll\Win32\Debug\indigo-renderer.dll lib\Win\x86\
copy ..\..\zlib-src\Win32\ReleaseDll\zlib.dll lib\Win\x86\

copy ..\..\api\dll\x64\Debug\indigo.dll lib\Win\x64\
copy ..\..\api\renderer\dll\x64\Debug\indigo-renderer.dll lib\Win\x64\
copy ..\..\zlib-src\x64\ReleaseDll\zlib.dll lib\Win\x64\