if "%ProgramFiles(x86)%" == "" goto L1
call "%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
goto L2
:L1
call "%ProgramFiles%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
:L2
devenv indigo-deco.sln /build "Release|x64"
devenv indigo-deco.sln /build "Release|Win32"
call release-windows.bat indigo-deco-%1-win32 Win32
call release-windows.bat indigo-deco-%1-win64 x64
