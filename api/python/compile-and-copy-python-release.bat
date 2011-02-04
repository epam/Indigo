if "%ProgramFiles(x86)%" == "" goto L1
call "%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
goto L2
:L1
call "%ProgramFiles%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
:L2
cd ..
devenv indigo-api.sln /build "ReleaseDLL|x64"
devenv indigo-api.sln /build "ReleaseDLL|Win32"
cd python

call copy-libs.bat