@echo off
mkdir lib\Win\x86
mkdir lib\Win\x64

copy ..\api\jni\Win32\Release\indigo-jni.dll lib\Win\x86\
copy ..\api\jni\x64\Release\indigo-jni.dll lib\Win\x64\
copy ..\api\renderer\jni\Win32\Release\indigo-renderer-jni.dll lib\Win\x86\
copy ..\api\renderer\jni\x64\Release\indigo-renderer-jni.dll lib\Win\x64\
copy ..\zlib-src\Win32\ReleaseDll\zlib.dll lib\Win\x86\
copy ..\zlib-src\x64\ReleaseDll\zlib.dll lib\Win\x64\