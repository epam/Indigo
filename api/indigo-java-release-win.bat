rd /S /Q %1

cd java
call compile.bat
cd ..\renderer\java
call compile.bat
cd ..\..\

mkdir %1\lib\Win\x86
mkdir %1\lib\Win\x64

copy LICENSE.GPL %1\
copy jni\Win32\Release\indigo-jni.dll %1\lib\Win\x86\
copy jni\x64\Release\indigo-jni.dll %1\lib\Win\x64\
copy renderer\jni\Win32\Release\indigo-renderer-jni.dll %1\lib\Win\x86\
copy renderer\jni\x64\Release\indigo-renderer-jni.dll %1\lib\Win\x64\
copy ..\zlib-src\Win32\ReleaseDll\zlib.dll %1\lib\Win\x86\
copy ..\zlib-src\x64\ReleaseDll\zlib.dll %1\lib\Win\x64\

copy java\dist\indigo-java.jar %1\
copy renderer\java\dist\indigo-renderer-java.jar %1\

zip -r -9 %1.zip %1

