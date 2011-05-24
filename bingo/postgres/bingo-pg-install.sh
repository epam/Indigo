#!/bin/sh
# Copyright (C) 2009-2011 GGA Software Services LLC
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

libdir=$PWD/bin
schema_name="bingo"
libext=".so"

usage ()
{
echo 'Usage: bingo-pg-install.sh [parameters]'
echo 'Parameters:'
echo '  -?, -help'
echo '    Print this help message'
echo '  -libdir path'
echo '    Target directory to install bingo_postgres'$libext' (defaut {CURRENT_DIR}/bin/).'
echo '    If the directory does not exist, it will be created.'
echo '  -schema name'
echo '    Postgres schema name (default "bingo").'
}


#if [ -f "../bin/libbingo.dylib" ]; then
#  libext=".dylib"
#fi

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
	   -schema)
        shift
        schema_name=$1
        ;;
     *)
        echo "Unknown parameter: $1";
        usage;
        exit -1
  esac
  shift
done

echo "Target directory  : $libdir";
echo "DBA schema name   : $schema_name";

if [ "$y" != "1" ]; then
  echo "Proceed (y/N)?"
  read proceed

  if [ "$proceed" != "y" ] && [ "$proceed" != "Y" ]; then
    echo 'Aborting';
    exit 0;
  fi
fi

if [ "$libdir" != "$PWD/bin" ]; then
  mkdir -p $libdir
  cp bin/bingo_postgres$libext $libdir
fi

if [ $? != 0 ]; then
  echo 'Cannot copy bingo_postgres'$libext' to '$libdir
  exit
fi

#echo set verify off >bingo/bingo_lib.sql 
#echo spool bingo_lib\; >>bingo/bingo_lib.sql 
#echo create or replace LIBRARY bingolib AS \'$libdir/bingo_postgres$libext\' >>bingo/bingo_lib.sql 
#echo / >>bingo/bingo_lib.sql 
#echo spool off\; >>bingo/bingo_lib.sql 



# Generate install script
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo/bingo_schema.sql.in  >bingo_install.sql

sed 's,BINGO_PATHNAME,'$libdir'/bingo_postgres,g' <sql/bingo/mango_internal.sql.in >>bingo_install.sql
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo/mango_pg.sql.in       >>bingo_install.sql

sed 's,BINGO_PATHNAME,'$libdir'/bingo_postgres,g' <sql/bingo/bingo_am.sql.in     >>bingo_install.sql
sed 's,BINGO_PATHNAME,'$libdir'/bingo_postgres,g' <sql/bingo/bingo_config.sql.in >>bingo_install.sql

#Generate uninstall script
sed 's,BINGO_SCHEMANAME,'$schema_name',g'         <sql/bingo/bingo_uninstall.quick.sql.in >bingo_uninstall.sql


#psql9.0 $dbaname -f bingo_postgres.sql

#cd system
#if [ "$dbapass" = "" ]; then
#  sqlplus $dbaname$instance @bingo_init.sql $bingoname $bingopass
#else
#  sqlplus $dbaname/$dbapass$instance @bingo_init.sql $bingoname $bingopass
#fi

#cd ../bingo
#sqlplus $bingoname/$bingopass$instance @makebingo.sql
#sqlplus $bingoname/$bingopass$instance @bingo_config.sql
#cd ..
#sqlplus $bingoname/$bingopass$instance @dbcheck.sql
