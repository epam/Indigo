@rem Copyright (C) from 2009 to Present EPAM Systems.
@rem 
@rem This file is part of Indigo toolkit.
@rem 
@rem Licensed under the Apache License, Version 2.0 (the "License");
@rem you may not use this file except in compliance with the License.
@rem You may obtain a copy of the License at
@rem 
@rem http://www.apache.org/licenses/LICENSE-2.0
@rem 
@rem Unless required by applicable law or agreed to in writing, software
@rem distributed under the License is distributed on an "AS IS" BASIS,
@rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@rem See the License for the specific language governing permissions and
@rem limitations under the License.
@echo off

set dbaname=
set dbapass=
set database=
set bingoname=bingo
set y=
set server=
set fulldelete=0

if exist bingo_saved_params.bat  call bingo_saved_params.bat

goto L2

:L1
shift

:L2
if "%1" == "" goto L3
if "%1" == "-dbaname" goto got_dbaname
if "%1" == "-dbapass" goto got_dbapass
if "%1" == "-database" goto got_database
if "%1" == "-bingoname" goto got_bingoname
if "%1" == "-server" goto got_server
if "%1" == "-y" goto got_y
if "%1" == "-fulldelete" goto got_fulldelete

if "%1" == "-help" goto usage
if "%1" == "-?" goto usage
if "%1" == "/?" goto usage
goto badparam

:got_dbaname
  shift
  set dbaname=%1
  goto L1

:got_dbapass
  shift
  set dbapass=%1
  goto L1

:got_database
  shift
  set database=%1
  goto L1

:got_server
  shift
  set server=%1
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

:got_fulldelete
  set fulldelete=1
  goto L1
  
:badparam
  echo Unknown parameter: %1
  goto end

:L3

if "%database%" == "" goto no_database

if "%dbaname%" == "" goto no_dbaname

:L5

goto confirm

:no_database
echo Database not specified (use -database)
goto end

:no_dbaname
if "%dbapass%" == "" goto L5
echo DBA password specfied without a DBA name (use -dbaname)
goto end

:confirm
if not "%dbaname%" == "" echo DBA name     : %dbaname%
if not "%dbapass%" == "" echo DBA password     : %dbapass%
if not "%server%" == "" echo Server            : %server%
echo Database          : %database%
echo Bingo name        : %bingoname%
if "%fulldelete%" == "1" echo Full delete       : True
if "%fulldelete%" == "0" echo Full delete       : False

if "%y%"=="1" goto L4

set /p proceed=Proceed (y/N)? 

if "%proceed%"=="y" goto L4
if "%proceed%"=="Y" goto L4

echo Aborting
goto end

:L4

set bingo_sqlcmd_arguments= -vbingo=%bingoname% -vdatabase=%database% -vfulldelete=%fulldelete%

if not "%dbaname%" == "" set bingo_sqlcmd_arguments=%bingo_sqlcmd_arguments% -U %dbaname% 
if not "%dbapass%" == "" set bingo_sqlcmd_arguments=%bingo_sqlcmd_arguments% -P %dbapass%
if not "%server%" == "" set bingo_sqlcmd_arguments=%bingo_sqlcmd_arguments% -S %server%

sqlcmd -i bingo_drop.sql %bingo_sqlcmd_arguments%
goto end

:usage
echo Usage: bingo-sqlserver-uninstall.bat [parameters]
echo Parameters:
echo   -?, -help
echo     Print this help message
echo   -server name
echo     SQL Server name (default is local SQL Server)
echo   -database database (obligatory)
echo     Database to remove Bingo from.
echo   -dbaname name
echo     Database administrator login (default is current user).
echo   -dbapass password
echo     Database administrator password.
echo     If the password is not specified, you will have to enter it later.
echo   -bingoname name
echo     Name of cartridge pseudo-user (default "bingo").
echo   -y
echo     Do not ask for confirmation.
echo   -fulldelete
echo     Delete all Bingo-dependant objects that are used between different Bingo installations.
goto end


:end

set dbaname=
set dbapass=
set instance=
set bingoname=
set bingopass=
set database=
set y=
set bingo_sqlcmd_arguments=
set server=
