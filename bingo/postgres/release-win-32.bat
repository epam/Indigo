@echo off
if "%1" == "" goto ERROR_0

if "%ProgramFiles(x86)%" == "" goto L1
call "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
goto L2
:L1
call "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
:L2

if "%BINGO_PG_DIR32%"=="" goto SET_DIR
goto CALL_BUILD

:SET_DIR
if "%BINGO_PG_DIR%" == "" goto ERROR_1
set BINGO_PG_DIR32=%BINGO_PG_DIR%
goto CALL_BUILD

:CALL_BUILD
devenv bingo_postgres.sln /build "Release|Win32"
bingo-release.bat %1 dist\x86\bingo_postgres.dll
goto EXIT_S

:ERROR_0
echo Please specify version
goto EXIT_S

:ERROR_1
echo BINGO_PG_DIR is not specified
goto EXIT_S

:EXIT_S

