@rem Copyright (C) 2009-2010 GGA Software Services LLC
@rem 
@rem This file is part of Indigo toolkit.
@rem 
@rem This file may be distributed and/or modified under the terms of the
@rem GNU General Public License version 3 as published by the Free Software
@rem Foundation and appearing in the file LICENSE.GPL included in the
@rem packaging of this file.
@rem 
@rem This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
@rem WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

@echo off
set libdir=%ORACLE_HOME%\bin
set dbaname=system
set dbapass=
set instance=
set bingoname=bingo
set bingopass=bingo
set y=
set nine=

goto L2

:L1
shift

:L2
if "%1" == "" goto L3
if "%1" == "-libdir" goto got_libdir
if "%1" == "-dbaname" goto got_dbaname
if "%1" == "-dbapass" goto got_dbapass
if "%1" == "-instance" goto got_instance
if "%1" == "-bingoname" goto got_bingoname
if "%1" == "-bingopass" goto got_bingopass
if "%1" == "-y" goto got_y
if "%1" == "-9" goto got_nine

if "%1" == "-help" goto usage
if "%1" == "-?" goto usage
if "%1" == "/?" goto usage
goto badparam

:got_libdir
  shift
  set libdir=%1
  goto L1

:got_dbaname
  shift
  set dbaname=%1
  goto L1

:got_dbapass
  shift
  set dbapass=%1
  goto L1

:got_instance
  shift
  set instance=%1
  goto L1

:got_bingoname
  shift
  set bingoname=%1
  goto L1

:got_bingopass
  shift
  set bingopass=%1
  goto L1

:got_y
  set y=1
  goto L1
  
:got_nine
  set nine=1
  goto L1

:badparam
  echo Unknown parameter: %1
  goto end

:L3

echo Target directory  : %libdir%
echo DBA name          : %dbaname%
if not "%dbapass%" == "" echo DBA password      : %DBApass%
if "%nine%"=="1" echo Oracle version    : 9
if "%nine%"==""  echo Oracle version    : 10 or 11
if "%instance%"=="" echo Oracle instance   : [default]
if not "%instance%"=="" echo Oracle instance   : %instance%
echo Bingo name        : %BINGOname%
echo Bingo password    : %BINGOpass%

if "%y%"=="1" goto L4

set /p proceed=Proceed (y/N)? 

if "%proceed%"=="y" goto L4
if "%proceed%"=="Y" goto L4

echo Aborting
goto end

:L4

echo set verify off >bingo\bingo_lib.sql 
echo spool bingo_lib; >>bingo\bingo_lib.sql 
echo create or replace LIBRARY bingolib AS '%libdir%\bingo-oracle.dll' >>bingo\bingo_lib.sql 
echo / >>bingo\bingo_lib.sql 
echo spool off; >>bingo\bingo_lib.sql 

md %libdir%
copy ..\bin\bingo-oracle.dll %libdir% /y
if not %errorlevel%==0 goto end

if "%nine%"=="" set initsql=bingo_init.sql
if "%nine%"=="1" set initsql=bingo_init_9.sql

if not "%instance%"=="" set instance=@%instance%

cd system
if "%dbapass%"=="" goto emptypass
sqlplus %dbaname%/%dbapass%%instance% @%initsql% %bingoname% %bingopass%
goto L5
:emptypass
sqlplus %dbaname%%instance% @%initsql% %bingoname% %bingopass%

:L5
cd ..\bingo
sqlplus %bingoname%/%bingopass%%instance% @makebingo.sql
sqlplus %bingoname%/%bingopass%%instance% @bingo_config.sql

cd ..
sqlplus %bingoname%/%bingopass%%instance% @dbcheck.sql

goto end

:usage
echo Usage: bingo-install.bat [parameters]
echo Parameters:
echo   -?, -help
echo     Print this help message
echo   -libdir path
echo     Target directory to install bingo-oracle.dll (defaut %%ORACLE_HOME%%\bin).
echo     If the directory does not exist, it will be created.
echo   -dbaname name
echo     Database administrator login (default "system").
echo   -dbapass password
echo     Database administrator password (no default).
echo     If the password is not specified, you will have to enter it later.
echo   -instance instance
echo     Database instance (default instance by default).
echo     You can specify full address like "server:1521/instance" as well.
echo   -bingoname name
echo     Name of cartridge pseudo-user (default "bingo").
echo   -bingopass password
echo     Password of the pseudo-user (default "bingo").
echo   -9
echo     Must be specified when installing on Oracle 9.
echo     Oracle 10 or 11 is assumed by default.
echo   -y
echo     Do not ask for confirmation.
goto end


:end

set libdir=
set dbaname=
set dbapass=
set instance=
set bingoname=
set bingopass=
set y=
set initsql=
