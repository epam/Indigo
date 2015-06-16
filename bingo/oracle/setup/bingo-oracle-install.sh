#!/bin/sh
# Copyright (C) 2009-2015 EPAM Systems
# 
# This file is part of Indigo toolkit.
# 
# This file may be distributed and/or modified under the terms of the
# GNU General Public License version 3 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.
# 
# This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
# WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

libdir=$ORACLE_HOME/lib
dbaname="system"
dbapass=
instance=
bingoname="bingo"
bingopass="bingo"
y=

usage ()
{
echo 'Usage: bingo-oracle-install.sh [parameters]'
echo 'Parameters:'
echo '  -?, -help'
echo '    Print this help message'
echo '  -libdir path'
echo '    Target directory to install libbingo-oracle'$libext' (defaut $ORACLE_HOME/lib).'
echo '    If the directory does not exist, it will be created.'
echo '  -dbaname name'
echo '    Database administrator login (default "system").'
echo '  -dbapass password'
echo '    Database administrator password (no default).'
echo '    If the password is not specified, you will have to enter it later.'
echo '  -instance instance'
echo '    Database instance (default instance by default).'
echo '    You can specify full address like "server:1521/instance" as well.'
echo '  -bingoname name'
echo '    Name of cartridge pseudo-user (default "bingo").'
echo '  -bingopass password'
echo '    Password of the pseudo-user (default "bingo").'
echo '  -y'
echo '    Do not ask for confirmation.'
}

libext=".so"
if [ -f "lib/libbingo-oracle.dylib" ]; then
  libext=".dylib"
fi

while [ "$#" != 0 ]; do
  case "$1" in
     -help | '-?' | '/?')
        usage
        exit 0
        ;;
     -libdir)
        shift
        libdir=$1
        ;;
	   -dbaname)
        shift
        dbaname=$1
        ;;
     -dbapass)
        shift
        dbapass=$1
        ;;
     -instance)
        shift
        instance=$1
        ;;
     -bingoname)
        shift
        bingoname=$1
        ;;
     -bingopass)
        shift
        bingopass=$1
        ;;
     -y)
        y=1
        ;;
     *)
        echo "Unknown parameter: $1";
        usage;
        exit -1
  esac
  shift
done

echo "Target directory  : $libdir";
echo "DBA name          : $dbaname";
if [ ! "$dbapass" = "" ]; then
  echo "DBA password      : $dbapass";
fi
if [ ! "$instance" = "" ]; then
  echo "Oracle instance   : $instance";
else
  echo "Oracle instance   : <default>";
fi
echo "Bingo name        : $bingoname";
echo "Bingo password    : $bingopass";

if [ "$y" != "1" ]; then
  echo "Proceed (y/N)?"
  read proceed

  if [ "$proceed" != "y" ] && [ "$proceed" != "Y" ]; then
    echo 'Aborting';
    exit 0;
  fi
fi

if [ ! "$instance" = "" ]; then
  instance=@$instance
fi

mkdir -p $libdir

echo set verify off >sql/bingo/bingo_lib.sql 
echo spool bingo_lib\; >>sql/bingo/bingo_lib.sql 
echo create or replace LIBRARY bingolib AS \'$libdir/libbingo-oracle$libext\' >>sql/bingo/bingo_lib.sql 
echo / >>sql/bingo/bingo_lib.sql 
echo spool off\; >>sql/bingo/bingo_lib.sql 

cp lib/libbingo-oracle$libext $libdir
if [ $? != 0 ]; then
  echo 'Cannot copy libbingo-oracle'$libext' to '$libdir
  exit
fi

cd sql/system
if [ "$dbapass" = "" ]; then
  sqlplus $dbaname$instance @bingo_init.sql $bingoname $bingopass
else
  sqlplus $dbaname/$dbapass$instance @bingo_init.sql $bingoname $bingopass
fi

cd ../bingo
sqlplus $bingoname/$bingopass$instance @makebingo.sql
sqlplus $bingoname/$bingopass$instance @bingo_config.sql
cd ..
sqlplus $bingoname/$bingopass$instance @dbcheck.sql
cd ..

