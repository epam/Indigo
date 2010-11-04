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

set dbaname=
set dbapass=
set database=
set bingoname=bingo
set y=
set server=

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

if "%y%"=="1" goto L4

set /p proceed=Proceed (y/N)? 

if "%proceed%"=="y" goto L4
if "%proceed%"=="Y" goto L4

echo Aborting
goto end

:L4

set bingo_sqlcmd_arguments= -vbingo=%bingoname% -vdatabase=%database%

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
goto end


:end

set libdir=
set dbaname=
set dbapass=
set instance=
set bingoname=
set bingopass=
set database=
set y=
set bingo_sqlcmd_arguments=
set server=
