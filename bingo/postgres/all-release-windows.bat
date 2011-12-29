@echo off 

if "%1" == "" goto ERROR_0
if "%2" == "" goto ERROR_1

call release-win-32.bat bingo-postgres%2-%1-win32 %2
call release-win-64.bat bingo-postgres%2-%1-win64 %2
goto EXIT_S

:ERROR_0
echo Please specify bingo version
goto EXIT_S

:ERROR_1
echo Please specify PostgreSQL version
goto EXIT_S

:EXIT_S
