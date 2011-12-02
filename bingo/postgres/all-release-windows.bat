@echo off 

if "%1" == "" goto ERROR_0

call release-win-32.bat bingo-postgres-%1-win32
call release-win-64.bat bingo-postgres-%1-win64
goto EXIT_S

:ERROR_0
echo Please specify version
goto EXIT_S

:EXIT_S
