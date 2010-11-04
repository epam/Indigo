rd /S /Q %1

mkdir %1\lib\Win\x86
mkdir %1\lib\Win\x64

copy LICENSE.GPL %1\
copy python\indigo.py %1\
copy renderer\python\indigo_renderer.py %1\
copy dll\Win32\Release\indigo.dll %1\lib\Win\x86\
copy dll\x64\Release\indigo.dll %1\lib\Win\x64\
copy renderer\dll\Win32\Release\indigo-renderer.dll %1\lib\Win\x86\
copy renderer\dll\x64\Release\indigo-renderer.dll %1\lib\Win\x64\
copy ..\zlib-src\Win32\ReleaseDll\zlib.dll %1\lib\Win\x86\
copy ..\zlib-src\x64\ReleaseDll\zlib.dll %1\lib\Win\x64\

zip -r -9 %1.zip %1
