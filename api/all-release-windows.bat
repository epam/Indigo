if "%ProgramFiles(x86)%" == "" goto L1
call "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
goto L2
:L1
call "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
:L2
devenv indigo-api.sln /build "Release|x64"
devenv indigo-api.sln /build "Release|Win32"
devenv indigo-api.sln /build "ReleaseDLL|x64"
devenv indigo-api.sln /build "ReleaseDLL|Win32"
devenv indigo-api.sln /build "ReleaseJNI|x64"
devenv indigo-api.sln /build "ReleaseJNI|Win32"
cd java
call compile.bat
cd ..\renderer\java
call compile.bat
cd ..\..
call indigo-java-release-win indigo-java-api-%1-windows
call indigo-python-release-win indigo-python-api-%1-windows
call indigo-libs-release-win indigo-libs-%1-win32 Win32
call indigo-libs-release-win indigo-libs-%1-win64 x64
call indigo-dotnet-release indigo-dotnet-%1
