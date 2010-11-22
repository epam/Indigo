mkdir lib\Win\x86
mkdir lib\Win\x64

cd ..\api

call "all-release-windows.bat"

cd ..\legio

copy ..\api\jni\Win32\Release\indigo-jni.dll lib\Win\x86\
copy ..\api\renderer\jni\Win32\Release\indigo-renderer-jni.dll lib\Win\x86\
copy ..\zlib-src\Win32\ReleaseDll\zlib.dll lib\Win\x86\