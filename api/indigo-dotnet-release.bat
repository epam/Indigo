rmdir /S /Q %1

mkdir %1\lib\Win\x86
mkdir %1\lib\Win\x64
mkdir %1\src

copy LICENSE.GPL %1\
copy dll\Win32\Release\indigo.dll %1\lib\Win\x86\
copy dll\x64\Release\indigo.dll %1\lib\Win\x64\
copy renderer\dll\Win32\Release\indigo-renderer.dll %1\lib\Win\x86\
copy renderer\dll\x64\Release\indigo-renderer.dll %1\lib\Win\x64\
copy ..\zlib-src\Win32\ReleaseDll\zlib.dll %1\lib\Win\x86\
copy ..\zlib-src\x64\ReleaseDll\zlib.dll %1\lib\Win\x64\
copy cs\Indigo.cs %1\src\
copy cs\IndigoObject.cs %1\src\
copy cs\IndigoException.cs %1\src\
copy cs\Indigo.snk %1\src\
copy renderer\cs\indigo-renderer.snk %1\src\
copy renderer\cs\IndigoRenderer.cs %1\src\
copy cs\bin\Release\indigo-cs.dll %1\
copy renderer\cs\bin\Release\indigo-renderer-cs.dll %1\

zip -r -9 %1.zip %1
