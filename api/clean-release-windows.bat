if "%ProgramFiles(x86)%" == "" goto L1
call "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
goto L2
:L1
call "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
:L2
devenv indigo-api.sln /clean "Release|x64"
devenv indigo-api.sln /clean "Release|Win32"
devenv indigo-api.sln /clean "ReleaseDLL|x64"
devenv indigo-api.sln /clean "ReleaseDLL|Win32"
devenv indigo-api.sln /clean "ReleaseDotNet|Any CPU"
