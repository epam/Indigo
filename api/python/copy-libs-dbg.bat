mkdir lib\Win\x86
mkdir lib\Win\x64
copy ..\dll\Win32\Debug\indigo.dll lib\Win\x86\
copy ..\dll\x64\Debug\indigo.dll lib\Win\x64\
copy ..\renderer\dll\Win32\Debug\indigo-renderer.dll lib\Win\x86\
copy ..\renderer\dll\x64\Debug\indigo-renderer.dll lib\Win\x64\
copy ..\..\zlib-src\Win32\ReleaseDll\zlib.dll lib\Win\x86\
copy ..\..\zlib-src\x64\ReleaseDll\zlib.dll lib\Win\x64\
