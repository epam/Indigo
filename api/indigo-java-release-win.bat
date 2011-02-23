rd /S /Q %1

cd java
call compile.bat
cd ..\renderer\java
call compile.bat
cd ..\..\

mkdir %1\lib\Win\x86
mkdir %1\lib\Win\x64

copy LICENSE.GPL %1\
copy dll\Win32\Release\indigo.dll %1\lib\Win\x86\
copy dll\x64\Release\indigo.dll %1\lib\Win\x64\
copy renderer\dll\Win32\Release\indigo-renderer.dll %1\lib\Win\x86\
copy renderer\dll\x64\Release\indigo-renderer.dll %1\lib\Win\x64\
copy ..\zlib-src\Win32\ReleaseDll\zlib.dll %1\lib\Win\x86\
copy ..\zlib-src\x64\ReleaseDll\zlib.dll %1\lib\Win\x64\

copy java\dist\indigo-java.jar %1\
copy renderer\java\dist\indigo-renderer-java.jar %1\
copy ..\common\jna\jna.jar %1\

zip -r -9 %1.zip %1
