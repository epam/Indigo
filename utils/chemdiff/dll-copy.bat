mkdir lib\Win\x86
mkdir lib\Win\x64

cd ..\..\api

call "all-release-windows.bat"

cd ..\utils\chemdiff

copy ..\..\api\jni\Win32\Release\indigo-jni.dll lib\Win\x86\