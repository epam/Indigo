mkdir lib\Win\x86
mkdir lib\Win\x64

cd ..\..\api

if "%ProgramFiles(x86)%" == "" goto L1
call "%ProgramFiles(x86)%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
goto L2
:L1
call "%ProgramFiles%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
:L2
devenv indigo-api.sln /build "ReleaseJNI|x64"
devenv indigo-api.sln /build "ReleaseJNI|Win32"

cd ..\utils\chemdiff
copy ..\..\api\jni\Win32\Release\indigo-jni.dll lib\Win\x86\
copy ..\..\api\renderer\jni\Win32\Release\indigo-renderer-jni.dll lib\Win\x86\
copy ..\..\zlib-src\Win32\ReleaseDll\zlib.dll lib\Win\x86\