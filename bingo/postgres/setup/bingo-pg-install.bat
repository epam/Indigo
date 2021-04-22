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
set libdir=%CD%/lib

set schemaname=bingo
set y=
set pglibdir=

goto L2

:L1
shift

:L2
if "%1" == "" goto L3
if "%1" == "-libdir" goto got_libdir
if "%1" == "-schemaname" goto got_schemaname
if "%1" == "-y" goto got_y
if "%1" == "-pglibdir" goto :got_pglibdir
if "%1" == "-help" goto usage
if "%1" == "-?" goto usage
if "%1" == "/?" goto usage
goto badparam

:got_libdir
  shift
  set libdir=%1
  goto L1

:got_schemaname
  shift
  set schemaname=%1
  goto L1

:got_y
  set y=1
  goto L1

:got_pglibdir
  set pglibdir=1
  goto L1
  
:badparam
  echo Unknown parameter: %1
  goto end
  
:L3

if "%pglibdir%"=="1" set libdir=$libdir

set libdir=%libdir:\=/%
echo Target directory  : %libdir%
echo Schema name       : %schemaname%

if "%y%"=="1" goto L4
set /p proceed=Proceed (y/N)? 
if "%proceed%"=="y" goto L4
if "%proceed%"=="Y" goto L4
goto end



:L4
@rem Delete install and uninstall scripts
if exist bingo_install.sql del bingo_install.sql
@rem Generate replace.vbs script
echo Dim FileName, OutputFileName, Find, ReplaceWith, FileContents, dFileContents >> replace.vbs
echo Find         	= WScript.Arguments(0) >> replace.vbs
echo ReplaceWith  	= WScript.Arguments(1) >> replace.vbs
echo FileName     	= WScript.Arguments(2) >> replace.vbs
echo OutputFileName 	= WScript.Arguments(3) >> replace.vbs
echo.  >> replace.vbs
echo FileContents = GetFile(FileName) >> replace.vbs
echo.  >> replace.vbs
echo dFileContents = replace(FileContents, Find, ReplaceWith, 1, -1, 1) >> replace.vbs
echo.  >> replace.vbs
echo WriteFile OutputFileName, dFileContents >> replace.vbs
echo.  >> replace.vbs
echo function GetFile(FileName) >> replace.vbs
echo   Dim FS, FileStream >> replace.vbs
echo   Set FS = CreateObject("Scripting.FileSystemObject") >> replace.vbs
echo     on error resume Next >> replace.vbs
echo     Set FileStream = FS.OpenTextFile(FileName) >> replace.vbs
echo     GetFile = FileStream.ReadAll >> replace.vbs
echo End Function >> replace.vbs
echo.  >> replace.vbs
echo function WriteFile(FileName, Contents) >> replace.vbs
echo   Dim OutStream, FS >> replace.vbs
echo.  >> replace.vbs
echo   on error resume Next >> replace.vbs
echo   Set FS = CreateObject("Scripting.FileSystemObject") >> replace.vbs
echo     Set OutStream = FS.OpenTextFile(FileName, 8, True) >> replace.vbs
echo     OutStream.Write Contents >> replace.vbs
echo End Function >> replace.vbs
@rem Generate install script
cscript //b replace.vbs BINGO_SCHEMANAME %schemaname% sql\bingo_schema.sql.in bingo_install.sql
cscript //b replace.vbs BINGO_SCHEMANAME %schemaname% sql\bingo_pg.sql.in bingo_install.sql
cscript //b replace.vbs BINGO_PATHNAME %libdir%/bingo-postgres sql\bingo_internal.sql.in bingo_install.sql
cscript //b replace.vbs BINGO_PATHNAME %libdir%/bingo-postgres sql\mango_internal.sql.in bingo_install.sql
cscript //b replace.vbs BINGO_SCHEMANAME %schemaname% sql\mango_pg.sql.in bingo_install.sql
cscript //b replace.vbs BINGO_PATHNAME %libdir%/bingo-postgres sql\ringo_internal.sql.in bingo_install.sql
cscript //b replace.vbs BINGO_SCHEMANAME %schemaname% sql\ringo_pg.sql.in bingo_install.sql
cscript //b replace.vbs BINGO_PATHNAME %libdir%/bingo-postgres sql\bingo_am.sql.in bingo_install.sql
cscript //b replace.vbs BINGO_PATHNAME %libdir%/bingo-postgres sql\bingo_config.sql.in bingo_install.sql
@rem Generate uninstall script
if exist bingo_uninstall.sql del bingo_uninstall.sql
cscript //b replace.vbs BINGO_SCHEMANAME %schemaname% sql\bingo_uninstall.quick.sql.in bingo_uninstall.sql 
@rem Delete replace.vbs script
del replace.vbs
goto L5

:L5
@rem Run install script
@rem psql...
goto end

:usage
echo Usage: bingo-pg-install.bat [parameters]
echo Parameters:
echo   -?, -help
echo     Print this help message
echo   -libdir path
echo     Target directory with the installed bingo-postgres.dll (defaut %CD%\bin).
echo   -schema name
echo     Postgres schema name (default "bingo").
echo   -pglibdir
echo     Use postgreSQL $libdir option (default "false")
echo     Notice: bingo-postgres.dll must be placed in the package library directory
echo   -y
echo     Process default options (default "false")
goto end

:end
set libdir=
set schemaname=
