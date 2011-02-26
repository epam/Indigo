mkdir lib\Win\x86
mkdir lib\Win\x64
copy ..\dll\Win32\Debug\indigo.dll lib\Win\x86\
copy ..\dll\Win32\Debug\indigo.pdb lib\Win\x86\
copy ..\dll\x64\Debug\indigo.dll lib\Win\x64\
copy ..\dll\x64\Debug\indigo.pdb lib\Win\x64\
copy ..\renderer\dll\Win32\Debug\indigo-renderer.dll lib\Win\x86\
copy ..\renderer\dll\x64\Debug\indigo-renderer.dll lib\Win\x64\
copy ..\..\zlib-src\Win32\ReleaseDll\zlib.dll lib\Win\x86\
copy ..\..\zlib-src\x64\ReleaseDll\zlib.dll lib\Win\x64\

if "%ProgramFiles(x86)%" == "" goto L1
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" lib\Win\x86\
copy "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll" lib\Win\x64\
goto L2
:L1
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\msvcr100.dll" lib\Win\x86\
copy "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll" lib\Win\x64\
:L2
